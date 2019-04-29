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
static JOB_SYSTEM_THREADLOCAL uint64_t tls_jobCount[2];
static JOB_SYSTEM_THREADLOCAL JobSystem::ThreadId tls_workerId;
static JOB_SYSTEM_THREADLOCAL JobSystem::Job *tls_jobPool[2];
JOB_SYSTEM_THREADLOCAL std::list<JobSystem::JobEventTrigger> JobSystem::tls_jobEventTriggers;

static volatile LONG JobCountInQueues[2];


static JobSystem::Context* GJobContext = nullptr;
size_t jobPoolBufferSizePerQueue = 0;


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

static void FinishJob(JobSystem::Job *job) {
	LONG unfinishedJobs = InterlockedDecrement(&job->unfinishedJobs);
	assert(unfinishedJobs >= 0);
	if (unfinishedJobs == 0 && job->parent) {
		FinishJob(job->parent);
	}
}



JobSystem::Context::Context(int numWorkerThreads, int maxJobsPerThread)
	: m_nextWorkerId(0)
	, m_numWorkerThreads(numWorkerThreads)
{
	maxJobsPerThread = nextPowerOfTwo(maxJobsPerThread);
	m_maxJobsPerThread = maxJobsPerThread;
	jobPoolBufferSizePerQueue = numWorkerThreads * maxJobsPerThread * sizeof(Job);
	auto jobPoolBufferSize = jobPoolBufferSizePerQueue * 2 + kCdsJobCacheLineBytes - 1;

	m_jobPoolBuffer = malloc(jobPoolBufferSize);

	for (int iQueueIndex = 0 ; iQueueIndex < 2 ; ++iQueueIndex)
	{
		JobCountInQueues[iQueueIndex] = 0;

		m_workerJobQueues[iQueueIndex] = new JobSystem::WorkStealingQueue*[numWorkerThreads];
		size_t queueBufferSize = JobSystem::WorkStealingQueue::BufferSize(maxJobsPerThread);
		m_queueEntryBuffer[iQueueIndex] = malloc(queueBufferSize * numWorkerThreads);
		for (int iWorker = 0; iWorker < numWorkerThreads; ++iWorker)
		{
			m_workerJobQueues[iQueueIndex][iWorker] = new JobSystem::WorkStealingQueue();
			int initError = m_workerJobQueues[iQueueIndex][iWorker]->Init(
				maxJobsPerThread,
				(void*)(intptr_t(m_queueEntryBuffer[iQueueIndex]) + iWorker * queueBufferSize),
				queueBufferSize);
			(void)initError;
			assert(initError == 0);
		}
	}
	
}

JobSystem::Context::~Context()
{
	for (int iQueueIndex = 0; iQueueIndex < 2; ++iQueueIndex)
	{
		for (int iWorker = 0; iWorker < m_numWorkerThreads; ++iWorker)
		{
			delete m_workerJobQueues[iQueueIndex][iWorker];
		}
		delete[] m_workerJobQueues[iQueueIndex];
		free(m_queueEntryBuffer[iQueueIndex]);
		free(m_jobPoolBuffer);
	}
}


JobSystem::Job * GetJob(int iQueueIndex)
{
	JobSystem::WorkStealingQueue *myQueue = tls_jobContext->m_workerJobQueues[iQueueIndex][tls_workerId];
	JobSystem::Job *job = myQueue->Pop();
	if (!job) {
		// this worker's queue is empty; try to steal a job from another thread
		//int victimOffset = 1 + (rand() % tls_jobContext->m_numWorkerThreads - 1);
		//int victimIndex = (tls_workerId + victimOffset) % tls_jobContext->m_numWorkerThreads;
		if (tls_workerId != JobSystem::ThreadType_MainThread)
		{
			JobSystem::WorkStealingQueue *victimQueue = tls_jobContext->m_workerJobQueues[iQueueIndex][JobSystem::ThreadType_MainThread/*victimIndex*/];
			job = victimQueue->Steal();
			if (!job) { // nothing to steal
				JOB_YIELD(); // TODO(cort): busy-wait bad, right? But there might be a job to steal in ANOTHER queue, so we should try again shortly.
				return nullptr;
			}
		}
	}
	return job;
}

JobSystem::ThreadId JobSystem::workerId(void)
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

	InterlockedDecrement(&JobCountInQueues[job->QueneIndex]);
	assert(JobCountInQueues[job->QueneIndex] >= 0);

#if 0
	char szBuff[512];
	sprintf_s(szBuff, 512, "Job Complete Quene Index : %d\r\n", job->QueneIndex);
	OutputDebugStringA(szBuff);
#endif
}

static void WorkingThreadProc(JobSystem::ThreadId InThreadType)
{
	int workerId = JobSystem::initWorker(InThreadType,GJobContext);
	while (true)
	{
		JobSystem::Job *pJob = nullptr;
		if (JobCountInQueues[0] > 0)
		{
			pJob = GetJob(0);
		}
		else if(JobCountInQueues[1] > 0)
		{
			pJob = GetJob(1);
		}
		
		if (pJob)
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

JobSystem::Context * JobSystem::Init(int maxJobsPerWorker)
{
	assert(!GJobContext);
	GJobContext = new Context(ThreadType_MaxThread, maxJobsPerWorker);

	for (int iThread = ThreadType_WorkingThread1; iThread < ThreadType_MaxThread; iThread += 1) {
		DWORD ThreadID;
		auto hThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WorkingThreadProc, (LPVOID)iThread, 0, &ThreadID);
		workers.push_back(hThreadHandle);
	}

	initWorker(ThreadType_MainThread,GJobContext);

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

int JobSystem::initWorker(ThreadId InThreadType,Context *ctx)
{
	tls_workerId = InThreadType;

	tls_jobContext = ctx;
	tls_jobCount[0] = 0;
	tls_jobCount[1] = 0;
	assert(tls_workerId < ctx->m_numWorkerThreads);
	void *jobPoolBufferAligned = (void*)((uintptr_t(ctx->m_jobPoolBuffer) + kCdsJobCacheLineBytes - 1) & ~(kCdsJobCacheLineBytes - 1));
	assert((uintptr_t(jobPoolBufferAligned) % kCdsJobCacheLineBytes) == 0);
	tls_jobPool[0] = (Job*)((unsigned char*)jobPoolBufferAligned + 0)+tls_workerId * ctx->m_maxJobsPerThread;
	tls_jobPool[1] = (Job*)((unsigned char*)jobPoolBufferAligned + jobPoolBufferSizePerQueue)+tls_workerId * ctx->m_maxJobsPerThread;
	return tls_workerId;
}

JobSystem::Job * JobSystem::AllocateJob(int iQueueIndex)
{
	// TODO(cort): no protection against over-allocation
	uint64_t index = tls_jobCount[iQueueIndex]++;
	return &tls_jobPool[iQueueIndex][index & (tls_jobContext->m_maxJobsPerThread - 1)];
}

JobSystem::Job * JobSystem::createJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes, int iQueueIndex)
{
	if (embeddedData != nullptr && embeddedDataBytes > kCdsJobPaddingBytes) {
		assert(0);
		return NULL;
	}
	Job* currentParent = parent;

	if (parent)
	{
		InterlockedIncrement(&parent->unfinishedJobs);
	}

	Job *job = AllocateJob(iQueueIndex);
	job->QueneIndex = iQueueIndex;
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

int JobSystem::enqueueJob(JobSystem::Job *job,int iQueueIndex)
{
	int pushError = tls_jobContext->m_workerJobQueues[iQueueIndex][tls_workerId]->Push(job);
	InterlockedIncrement(&JobCountInQueues[iQueueIndex]);
	return pushError;
}

void JobSystem::waitForJob(const JobSystem::Job *job)
{
	while (!JobSystem::IsJobComplete(job)) {
		Job *nextJob = GetJob(0);
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

#if 0
			char szBuff[512];
			sprintf_s(szBuff, 512, "WorkStealingQueue::Steal ByWorkId : %d\r\n", JobSystem::workerId());
			OutputDebugStringA(szBuff);
#endif
			return job;
		}
	}
	return nullptr; // queue empty
}
