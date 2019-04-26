#include "stdafx.h"
#include "JobSystem.h"
#include <thread>
#include <assert.h>

#ifdef _MSC_VER
#   include <windows.h>
#   define JOB_YIELD() YieldProcessor()
#   define JOB_COMPILER_BARRIER _ReadWriteBarrier()
#   define JOB_MEMORY_BARRIER std::atomic_thread_fence(std::memory_order_seq_cst);
#else
#   include <emmintrin.h>
#   define JOB_YIELD() _mm_pause()
#   define JOB_COMPILER_BARRIER asm volatile("" ::: "memory")
#   define JOB_MEMORY_BARRIER asm volatile("mfence" ::: "memory")
#endif

static_assert((sizeof(struct JobSystem::Job) % kCdsJobCacheLineBytes) == 0, "Job struct is not cache-line-aligned!");


static JOB_SYSTEM_THREADLOCAL JobSystem::Context *tls_jobContext = nullptr;
static JOB_SYSTEM_THREADLOCAL uint64_t tls_jobCount = 0;
static JOB_SYSTEM_THREADLOCAL int tls_workerId = -1;
static JOB_SYSTEM_THREADLOCAL JobSystem::Job *tls_jobPool = nullptr;
JOB_SYSTEM_THREADLOCAL std::list<JobSystem::JobEventTrigger> JobSystem::tls_jobEventTriggers;

static inline uint32_t nextPowerOfTwo(uint32_t x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

static inline JobSystem::Job *AllocateJob() {
	// TODO(cort): no protection against over-allocation
	uint64_t index = tls_jobCount++;
	return &tls_jobPool[index & (tls_jobContext->m_maxJobsPerThread - 1)];
}

static void FinishJob(JobSystem::Job *job) {
	const int32_t unfinishedJobs = --(job->unfinishedJobs);
	assert(unfinishedJobs >= 0);
	if (unfinishedJobs == 0 && job->parent) {
		FinishJob(job->parent);
	}
}



JobSystem::Context::Context(int numWorkerThreads, int maxJobsPerThread)
	: m_workerJobQueues(nullptr)
	, m_nextWorkerId(0)
	, m_numWorkerThreads(numWorkerThreads)
{
	maxJobsPerThread = nextPowerOfTwo(maxJobsPerThread);
	m_maxJobsPerThread = maxJobsPerThread;

	m_workerJobQueues = new JobSystem::WorkStealingQueue*[numWorkerThreads];
	const size_t jobPoolBufferSize = numWorkerThreads * maxJobsPerThread * sizeof(Job) + kCdsJobCacheLineBytes - 1;
	m_jobPoolBuffer = malloc(jobPoolBufferSize);
	size_t queueBufferSize = JobSystem::WorkStealingQueue::BufferSize(maxJobsPerThread);
	m_queueEntryBuffer = malloc(queueBufferSize * numWorkerThreads);
	for (int iWorker = 0; iWorker < numWorkerThreads; ++iWorker)
	{
		m_workerJobQueues[iWorker] = new JobSystem::WorkStealingQueue();
		int initError = m_workerJobQueues[iWorker]->Init(
			maxJobsPerThread,
			(void*)(intptr_t(m_queueEntryBuffer) + iWorker * queueBufferSize),
			queueBufferSize);
		(void)initError;
		assert(initError == 0);
	}
}

JobSystem::Context::~Context()
{
	for (int iWorker = 0; iWorker < m_numWorkerThreads; ++iWorker)
	{
		delete m_workerJobQueues[iWorker];
	}
	delete[] m_workerJobQueues;
	free(m_queueEntryBuffer);
	free(m_jobPoolBuffer);
}


JobSystem::Job * GetJob(void)
{
	JobSystem::WorkStealingQueue *myQueue = tls_jobContext->m_workerJobQueues[tls_workerId];
	JobSystem::Job *job = myQueue->Pop();
	if (!job) {
		// this worker's queue is empty; try to steal a job from another thread
		int victimOffset = 1 + (rand() % tls_jobContext->m_numWorkerThreads - 1);
		int victimIndex = (tls_workerId + victimOffset) % tls_jobContext->m_numWorkerThreads;
		JobSystem::WorkStealingQueue *victimQueue = tls_jobContext->m_workerJobQueues[victimIndex];
		job = victimQueue->Steal();
		if (!job) { // nothing to steal
			JOB_YIELD(); // TODO(cort): busy-wait bad, right? But there might be a job to steal in ANOTHER queue, so we should try again shortly.
			return nullptr;
		}
	}
	return job;
}

int JobSystem::workerId(void)
{
	return tls_workerId;
}

bool JobSystem::IsJobComplete(const Job *job)
{
	return (job->unfinishedJobs == 0);
}



static void ExecuteJob(JobSystem::Job *job)
{
	(job->function)(job, job->data);
	FinishJob(job);
}

static void WorkingThreadProc(JobSystem::Context *jobCtx)
{
	int workerId = JobSystem::initWorker(jobCtx);
	while (true)
	{
		if (auto pJob = GetJob())
		{
			ExecuteJob(pJob);
		}
		else
		{
			::Sleep(1);
		}
	}
}

std::vector<HANDLE> workers;
JobSystem::Context* GJobContext = nullptr;
JobSystem::Context * JobSystem::Init(int numWorkers, int numWorkersToSpawn, int maxJobsPerWorker)
{
	assert(!GJobContext);
	GJobContext = new Context(numWorkers, maxJobsPerWorker);

	for (int iThread = 0; iThread < numWorkersToSpawn; iThread += 1) {
		DWORD ThreadID;
		auto hThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WorkingThreadProc, (LPVOID)GJobContext, 0, &ThreadID);
		workers.push_back(hThreadHandle);
	}

	initWorker(GJobContext);

	return GJobContext;
}

void JobSystem::UnInit()
{
// 	for (int iThread = 0; iThread < workers.size(); iThread += 1) {
// 		workers[iThread].detach();
// 	}

// 	if (GJobContext)
// 	{
// 		delete GJobContext;
// 		GJobContext = nullptr;
// 	}
}

void JobSystem::Update()
{
	// update all event Triggers
	for (auto It = tls_jobEventTriggers.begin() ; It != tls_jobEventTriggers.end() ;)
	{
		if (JobSystem::IsJobComplete(It->job))
		{
			It->_OnFinishEventWrapperBase->OnTrigger(It->job->data);
			tls_jobEventTriggers.erase(It++);
		}
		else
		{
			++It;
		}
	}

// 	std::list< JobEventTrigger>::iterator itList;
// 	for (itList = tls_jobEventTriggers.begin(); itList != tls_jobEventTriggers.end(); )
// 	{
// 		if (JobSystem::IsJobComplete(itList->job))
// 		{
// 			tls_jobEventTriggers.erase(itList++);
// 		}
// 		else
// 			itList++;
// 	}
}

int JobSystem::initWorker(Context *ctx)
{
	tls_jobContext = ctx;
	tls_jobCount = 0;
	tls_workerId = ctx->m_nextWorkerId++;
	assert(tls_workerId < ctx->m_numWorkerThreads);
	void *jobPoolBufferAligned = (void*)((uintptr_t(ctx->m_jobPoolBuffer) + kCdsJobCacheLineBytes - 1) & ~(kCdsJobCacheLineBytes - 1));
	assert((uintptr_t(jobPoolBufferAligned) % kCdsJobCacheLineBytes) == 0);
	tls_jobPool = (Job*)(jobPoolBufferAligned)+tls_workerId * ctx->m_maxJobsPerThread;
	return tls_workerId;
}

JobSystem::Job * JobSystem::AllocateJob()
{
	// TODO(cort): no protection against over-allocation
	uint64_t index = tls_jobCount++;
	return &tls_jobPool[index & (tls_jobContext->m_maxJobsPerThread - 1)];
}

JobSystem::Job * JobSystem::createJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes)
{
	if (embeddedData != nullptr && embeddedDataBytes > kCdsJobPaddingBytes) {
		assert(0);
		return NULL;
	}
	if (parent) {
		parent->unfinishedJobs++;
	}
	Job *job = AllocateJob();
	job->function = function;
	job->parent = parent;
	job->unfinishedJobs = 1;
	if (embeddedData) {
		assert(kCdsJobPaddingBytes > embeddedDataBytes);
		memcpy(job->padding, embeddedData, embeddedDataBytes);
		job->data = job->padding;
	}
	else {
		job->data = nullptr;
	}

	return job;
}

int JobSystem::enqueueJob(JobSystem::Job *job)
{
	int pushError = tls_jobContext->m_workerJobQueues[tls_workerId]->Push(job);
	return pushError;
}

void JobSystem::waitForJob(const JobSystem::Job *job)
{
	while (!JobSystem::IsJobComplete(job)) {
		Job *nextJob = GetJob();
		if (nextJob) {
			ExecuteJob(nextJob);
		}
		::Sleep(0);
	}
}




int JobSystem::WorkStealingQueue::Init(int capacity, void *buffer, size_t bufferSize)
{
	if ((capacity & (capacity - 1)) != 0) {
		return -2; // capacity must be a power of 2
	}
	size_t minBufferSize = BufferSize(capacity);
	if (bufferSize < minBufferSize) {
		return -1; // inadequate buffer size
	}
	uint8_t *bufferNext = (uint8_t*)buffer;
	m_entries = (Job**)bufferNext;
	bufferNext += capacity * sizeof(Job*);
	assert(bufferNext - (uint8_t*)buffer == (intptr_t)minBufferSize);

	for (int iEntry = 0; iEntry < capacity; iEntry += 1) {
		m_entries[iEntry] = nullptr;
	}

	m_top = 0;
	m_bottom = 0;
	m_capacity = capacity;

	return 0;
}

int JobSystem::WorkStealingQueue::Push(Job *job)
{
	// TODO: assert that this is only ever called by the owning thread
	uint64_t jobIndex = m_bottom;
	m_entries[jobIndex & (m_capacity - 1)] = job;

	// Ensure the job is written before the m_bottom increment is published.
	// A StoreStore memory barrier would also be necessary on platforms with a weak memory model.
	JOB_COMPILER_BARRIER;

	m_bottom = jobIndex + 1;
	return 0;
}

JobSystem::Job * JobSystem::WorkStealingQueue::Pop()
{
	// TODO: assert that this is only ever called by the owning thread
	uint64_t bottom = m_bottom - 1;
	m_bottom = bottom;

	// Make sure m_bottom is published before reading top.
	// Requires a full StoreLoad memory barrier, even on x86/64.
	JOB_MEMORY_BARRIER;

	uint64_t top = m_top;
	if ((int64_t)top <= (int64_t)bottom)
	{
		Job *job = m_entries[bottom & (m_capacity - 1)];
		if (top != bottom) {
			// still >0 jobs left in the queue
			return job;
		}
		else {
			// popping the last element in the queue
			if (!std::atomic_compare_exchange_strong(&m_top, &top, top + 1)) {
				// failed race against Steal()
				job = nullptr;
			}
			m_bottom = top + 1;

			return job;
		}
	}
	else
	{
		// queue already empty
		m_bottom = top;
		return nullptr;
	}
}

JobSystem::Job * JobSystem::WorkStealingQueue::Steal()
{
	// TODO: assert that this is never called by the owning thread
	uint64_t top = m_top;

	// Ensure top is always read before bottom.
	// A LoadLoad memory barrier would also be necessary on platforms with a weak memory model.
	JOB_COMPILER_BARRIER;

	uint64_t bottom = m_bottom;
	if (top < bottom) {
		Job *job = m_entries[top & (m_capacity - 1)];
		if (job)
		{
			// CAS serves as a compiler barrier as-is.
			if (!std::atomic_compare_exchange_strong(&m_top, &top, top + 1)) {
				// concurrent Steal()/Pop() got this entry first.
				return nullptr;
			}
			m_entries[top & (m_capacity - 1)] = nullptr;

			char szBuff[512];
			sprintf_s(szBuff, 512, "WorkStealingQueue::Steal ByWorkId : %d\r\n", JobSystem::workerId());
			OutputDebugStringA(szBuff);
			return job;
		}
	}
	return nullptr; // queue empty
}
