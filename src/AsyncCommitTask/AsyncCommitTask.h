// AsyncCommitTask.h

#ifndef _ASYNCCOMMITTASK_h
#define _ASYNCCOMMITTASK_h

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include <IEmbedded.h>

class AsyncCommitTask : public Task, public virtual IEmbeddedDataAsyncCommit
{
private:
	static const uint32_t InitialDelay = 1000;
	static const uint32_t NextPendingDelay = 200;

	IEmbeddedStorage* AsyncStorage = nullptr;

public:
	AsyncCommitTask(Scheduler* scheduler, IEmbeddedStorage* asyncStorage)
		: Task(0, TASK_FOREVER, scheduler, false)
		, AsyncStorage(asyncStorage)
	{
	}

	virtual void AsyncUpdate()
	{
		Task::enableIfNot();
	}

	bool OnEnable()
	{
		Task::delay(InitialDelay);

		return true;
	}

	bool Callback()
	{
		if (AsyncStorage->CommitNextPending())
		{
			Task::delay(NextPendingDelay);
		}
		else
		{
			Task::disable();
		}

		return true;
	}
};
#endif