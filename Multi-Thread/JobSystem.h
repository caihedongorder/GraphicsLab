#pragma once
#ifndef JOB_SYSTEM_H
#define JOB_SYSTEM_H
#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <vector>
#include <assert.h>
#include <memory>
#include <list>

typedef void(*OnFinishEventFunc)(const void*);

template<typename lambdaFunc>
OnFinishEventFunc defineFinishEvent(lambdaFunc InFunc) {
	return InFunc;
}


namespace JobSystem
{
	#define kCdsJobCacheLineBytes 64
	#define kCdsJobPaddingBytes ( (kCdsJobCacheLineBytes) - (sizeof(JobFunction) + sizeof(struct Job*) + sizeof(void*) + sizeof(std::atomic_int_fast32_t)) )

	#ifdef _MSC_VER
	#   define JOB_ATTR_ALIGN(alignment) __declspec(align(alignment))
	#else
	#   define JOB_ATTR_ALIGN(alignment) __attribute__((aligned(alignment)))
	#endif


	#if   defined(_MSC_VER)
	#   if _MSC_VER < 1900
	#       define JOB_SYSTEM_THREADLOCAL __declspec(thread)
	#   else
	#       define JOB_SYSTEM_THREADLOCAL thread_local
	#   endif
	#elif defined(__GNUC__)
	#   define JOB_SYSTEM_THREADLOCAL __thread
	#elif defined(__clang__)
	#   if defined(__APPLE__) || defined(__MACH__)
	#       define JOB_SYSTEM_THREADLOCAL __thread
	#   else
	#       define JOB_SYSTEM_THREADLOCAL thread_local
	#   endif
	#endif

	enum ThreadId
	{
		ThreadType_MainThread,
		ThreadType_MaxProduceThread,

		ThreadType_WorkingThread1 = ThreadType_MaxProduceThread,
		ThreadType_WorkingThread2,
		ThreadType_WorkingThread3,
		ThreadType_MaxThread = 12,
	};


	struct Job;
	class Context;
	typedef void(*JobFunction)(struct Job*, const void*);

	typedef JOB_ATTR_ALIGN(kCdsJobCacheLineBytes) struct Job {
		JobFunction function;
		struct Job *parent;
		void *data;
		volatile LONG unfinishedJobs;
		char padding[kCdsJobPaddingBytes];
	} Job;

	class OnFinishEventWrapperBase
	{
	public:
		OnFinishEventWrapperBase() {}
		virtual ~OnFinishEventWrapperBase() {}

		virtual void OnTrigger(void* InData) {}
	};

	template<typename OnFinishEvent>
	class TOnFinishEventWrapper : public OnFinishEventWrapperBase
	{
	public:
		TOnFinishEventWrapper(OnFinishEvent InOnFinishEvent)
			:_OnFinishEvent(InOnFinishEvent)
		{
		}
		virtual void OnTrigger(void* InData) override{
			_OnFinishEvent(InData);
		}
	private:
		OnFinishEvent _OnFinishEvent;
	};

	struct JobEventTrigger
	{
		JobSystem::Job *job;
		std::shared_ptr<OnFinishEventWrapperBase> _OnFinishEventWrapperBase;
	};
	extern JOB_SYSTEM_THREADLOCAL std::list<JobEventTrigger> tls_jobEventTriggers;
	extern LONG GJobCount;

	class WorkStealingQueue {
	public:
		static size_t BufferSize(int capacity) {
			return capacity * sizeof(Job*);
		}

		int Init(int capacity, void *buffer, size_t bufferSize);
		int Push(Job *job);
		Job *Pop();
		Job *Steal();

	private:
		Job **m_entries;
		std::atomic<uint64_t> m_top;
		uint64_t m_bottom;
		int m_capacity;
	};

	class Context 
	{
	private:
		Context() = delete;
		Context(const Context &ctx) = delete;
	public:
		Context(int numWorkerThreads, int maxJobsPerThread);
		~Context();

		WorkStealingQueue **m_workerJobQueues[2];
		void *m_jobPoolBuffer[2];
		void *m_queueEntryBuffer[2];
		std::atomic<int> m_nextWorkerId;
		int m_numWorkerThreads;
		int m_maxJobsPerThread;
	};

	// Called by main thread to create the shared job context for a pool of worker threads.
	Context *Init(int maxJobsPerWorker);

	void UnInit();

	void Update();


	// Called by each worker thread.
	int initWorker(ThreadId InThreadType,Context *ctx);


	static Job *AllocateJob();

	// Called by worker threads to create a new job to execute. This function does *not* enqueue the new job for execution.
	Job *createJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes);

	template<typename OnFinishFunc>
	Job *TCreateJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes, OnFinishFunc InOnFinishEventFunc,bool bEnqueue)
	{
		auto job = createJob(function, parent, embeddedData, embeddedDataBytes);

		if (bEnqueue)
			enqueueJob(job);

		JobEventTrigger eventTrigger;
		eventTrigger.job = job;
		eventTrigger._OnFinishEventWrapperBase = std::shared_ptr<OnFinishEventWrapperBase>(new TOnFinishEventWrapper<OnFinishFunc>(InOnFinishEventFunc));
		tls_jobEventTriggers.push_back(eventTrigger);

		return job;
	}


	// Called by worker threads to enqueue a job for execution. This gives the next available thread permission to execute this
	// job. All prior dependencies must be complete before a job is enqueued.
	int enqueueJob(Job *job, int iQueueIndex = 1);

	// Fetch and run any available queued jobs until the specified job is complete.
	void waitForJob(const Job *job);

	// Return the worker ID of the calling thread. If initWorker()
	// was called by this thread, the worker ID will be an index
	// from [0..numWorkers-1]. Otherwise, the worker ID is undefined.

	ThreadId workerId(void);
// 
// 	inline LONG GetJobCount()
// 	{
// #if 0
// 		char szBuff[512];
// 		sprintf_s(szBuff, 512, "GJobCount : %d\r\n", GJobCount);
// 		OutputDebugStringA(szBuff);
// #endif
// 		return GJobCount;
// 	}

	bool IsJobComplete(const Job *job);

	template <typename T, typename Func>
	struct SimpleJobData {
		typedef T DataType;

		SimpleJobData(DataType* data, Func InFunc)
			: data(data)
			, function(InFunc)
		{
		}

		DataType* data;
		Func function;
	};
	template <typename JobData>
	void simpleJobFunc(struct Job* job, const void* jobData) {
		const JobData* data = static_cast<const JobData*>(jobData);
			// execute the function on the range of data
		(data->function)(data->data);
	}

	template <typename T,typename Func, typename OnFinishFunction>
	Job* createSimpleJob(T* data, Func InFunc, OnFinishFunction InOnFinishEventFunc, bool bEnqueue,Job *parent = nullptr)
	{
		typedef SimpleJobData<T,Func> JobData;
		const JobData jobData(data, InFunc);
		return TCreateJob(simpleJobFunc<JobData>, parent, &jobData, sizeof(jobData), InOnFinishEventFunc,bEnqueue);
	}

	template <typename T, typename Func, typename OnFinishFunction>
	Job* createParallelForJob(T* data, int count, void *userData, Func InFunc,int InNumsSplite , Job *parent, OnFinishFunction InOnFinishEventFunc)
	{
		if (count < InNumsSplite)
		{
			return createSimpleJob(data, [=](T* InData) {
				return InFunc(InData, count, userData);
			}, InOnFinishEventFunc,true,parent);
		}
		auto ParallelJob = createSimpleJob(data, [=](T* InData) {}, InOnFinishEventFunc,false, parent);
		int SpliteCount = (count + InNumsSplite - 1) / InNumsSplite;
		std::vector<Job*> AllSubJobs;
		for (int SpliteIdx = 0 ; SpliteIdx < SpliteCount; ++SpliteIdx)
		{
			Job* pSpliteJob = nullptr;
			if (SpliteIdx == SpliteCount -1)
			{
				pSpliteJob = createSimpleJob(data+ SpliteIdx* InNumsSplite, [=](T* InData) {
					return InFunc(InData, count - SpliteIdx* InNumsSplite, userData);
				}, InOnFinishEventFunc,false, ParallelJob);
			}
			else
			{
				pSpliteJob = createSimpleJob(data + SpliteIdx * InNumsSplite, [=](T* InData) {
					return InFunc(InData, InNumsSplite, userData);
				}, InOnFinishEventFunc, false, ParallelJob);
			}

			AllSubJobs.push_back(pSpliteJob);
		}
		enqueueJob(ParallelJob);
		for (unsigned int iJboIdx = 0 ; iJboIdx < AllSubJobs.size() ; ++iJboIdx)
		{
			enqueueJob(AllSubJobs[iJboIdx]);
		}

		return ParallelJob;
	}
}
#endif

