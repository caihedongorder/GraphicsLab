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


	struct Job;
	class Context;
	typedef void(*JobFunction)(struct Job*, const void*);

	typedef JOB_ATTR_ALIGN(kCdsJobCacheLineBytes) struct Job {
		JobFunction function;
		struct Job *parent;
		void *data;
		std::atomic_int_fast32_t unfinishedJobs;
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

		WorkStealingQueue **m_workerJobQueues;
		void *m_jobPoolBuffer;
		void *m_queueEntryBuffer;
		std::atomic<int> m_nextWorkerId;
		int m_numWorkerThreads;
		int m_maxJobsPerThread;
	};

	// Called by main thread to create the shared job context for a pool of worker threads.
	Context *Init(int numWorkers, int numWorkersToSpawn, int maxJobsPerWorker);

	void UnInit();

	void Update();


	// Called by each worker thread.
	int initWorker(Context *ctx);


	static Job *AllocateJob();

	// Called by worker threads to create a new job to execute. This function does *not* enqueue the new job for execution.
	Job *createJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes);

	template<typename OnFinishFunc>
	Job *TCreateJob(JobFunction function, Job *parent, const void *embeddedData, size_t embeddedDataBytes, OnFinishFunc InOnFinishEventFunc)
	{
		auto job = createJob(function, parent, embeddedData, embeddedDataBytes);

		JobEventTrigger eventTrigger;
		eventTrigger.job = job;
		eventTrigger._OnFinishEventWrapperBase = std::shared_ptr<OnFinishEventWrapperBase>(new TOnFinishEventWrapper<OnFinishFunc>(InOnFinishEventFunc));
		tls_jobEventTriggers.push_back(eventTrigger);

		return job;
	}


	// Called by worker threads to enqueue a job for execution. This gives the next available thread permission to execute this
	// job. All prior dependencies must be complete before a job is enqueued.
	int enqueueJob(Job *job);

	// Fetch and run any available queued jobs until the specified job is complete.
	void waitForJob(const Job *job);

	// Return the worker ID of the calling thread. If initWorker()
	// was called by this thread, the worker ID will be an index
	// from [0..numWorkers-1]. Otherwise, the worker ID is undefined.
	int workerId(void);

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
	Job* createSimpleJob(T* data, Func InFunc, OnFinishFunction InOnFinishEventFunc,Job *parent = nullptr)
	{
		typedef SimpleJobData<T,Func> JobData;
		const JobData jobData(data, InFunc);
		return TCreateJob(simpleJobFunc<JobData>, parent, &jobData, sizeof(jobData), InOnFinishEventFunc);
	}

	template <typename T, typename S,typename Func>
	struct ParallelForJobData {
		typedef T DataType;
		typedef S SplitterType;

		ParallelForJobData(DataType* data, unsigned int count, void *userData, Func InFunc, const SplitterType& splitter)
			: data(data)
			, userData(userData)
			, function(InFunc)
			, splitter(splitter)
			, count(count)
		{
		}

		DataType* data;
		void *userData;
		Func function;
		SplitterType splitter;
		unsigned int count;
	};
	template <typename JobData>
	void parallelForJobFunc(struct Job* job, const void* jobData) {
		const JobData* data = static_cast<const JobData*>(jobData);
		const JobData::SplitterType& splitter = data->splitter;
		if (splitter.split<JobData::DataType>(data->count)) {
#if 0
			char szBuff[512];
			sprintf_s(szBuff, 512, "parallelForJobFunc: Parent Addr:%x\n", data->data);
			OutputDebugStringA(szBuff);
#endif

			// split in two
			const unsigned int leftCount = data->count / 2U;
			const JobData leftData(data->data + 0, leftCount, data->userData, data->function, splitter);
			Job *leftJob = createJob(parallelForJobFunc<JobData>, job, &leftData, sizeof(leftData));
			enqueueJob(leftJob);
#if 0
			sprintf_s(szBuff, 512, "parallelForJobFunc: leftJob Addr:%x\n", static_cast<const JobData*>(leftJob->data)->data);
			OutputDebugStringA(szBuff);
#endif

			const unsigned int rightCount = data->count - leftCount;
			const JobData rightData(data->data + leftCount, rightCount, data->userData, data->function, splitter);
			Job *rightJob = createJob(parallelForJobFunc<JobData>, job, &rightData, sizeof(rightData));
			enqueueJob(rightJob);

#if 0
			sprintf_s(szBuff, 512, "parallelForJobFunc: rightJob Addr:%x\n", static_cast<const JobData*>(rightJob->data)->data);
			OutputDebugStringA(szBuff);
#endif
		}
		else {
			// execute the function on the range of data
			(data->function)(data->data, data->count, data->userData);

#if 0
			char szBuff[512];
			sprintf_s(szBuff, 512, "parallelForJobFunc: Execute Addr:%x\n", data->data);
			OutputDebugStringA(szBuff);
#endif
		}
	}

	template <typename T, typename S, typename Func, typename OnFinishFunction>
	Job* createParallelForJob(T* data, unsigned int count, void *userData, Func InFunc,
		const S& splitter, Job *parent, OnFinishFunction InOnFinishEventFunc)
	{
		typedef ParallelForJobData<T, S, Func> JobData;
		const JobData jobData(data, count, userData, InFunc, splitter);
		return TCreateJob(parallelForJobFunc<JobData>, parent, &jobData, sizeof(jobData), InOnFinishEventFunc);
	}


	class CountSplitter {
	public:
		explicit CountSplitter(unsigned int count) : m_count(count) {}
		template <typename T> inline bool split(unsigned int count) const { return (count > m_count); }
	private:
		unsigned int m_count;
	};

	class DataSizeSplitter {
	public:
		explicit DataSizeSplitter(unsigned int size) : m_size(size) {}
		template <typename T> inline bool split(unsigned int count) const { return (count * sizeof(T) > m_size); }
	private:
		unsigned int m_size;
	};
}
#endif

