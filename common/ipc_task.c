#include "ipc_task.h"

#include "ipc_log.h"

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX(X, Y) 	((X > Y) ? X : Y)

#ifndef USEC_PER_SEC 
#define USEC_PER_SEC 1000000
#endif

struct IPC_Task_s
{
	struct IPC_Task_s* mPrev;
	struct IPC_Task_s* mNext;

	const char*   mName;
	IPC_Task_Proc mProc;

	int 		  mFd;
	void* 	      mParam;
};


typedef struct IPC_TaskList_s* IPC_TaskList;

struct IPC_TaskList_s
{
	IPC_Task mHead;
	IPC_Task mTail;
	int		 mCnt;
};

static struct IPC_TaskList_s _taskList;

static void 
_ipc_task_list_add(IPC_TaskList list, IPC_Task task)
{
	task->mNext = NULL;
	task->mPrev = list->mTail;
	if(list->mTail)
		list->mTail->mNext = task;
	else
		list->mHead = task;

	list->mTail = task;
	list->mCnt++;
}

static void 
_ipc_task_list_delete(IPC_TaskList list, IPC_Task task)
{
	
	if(task->mNext)
		task->mNext->mPrev = task->mPrev;
	else
		list->mTail = task->mPrev;
	
	if(task->mPrev)
		task->mPrev->mNext = task->mNext;
	else
		list->mHead = task->mNext;

	task->mNext =task->mPrev = NULL;

	list->mCnt --;
}

static void
_ipc_task_list_delete_all(IPC_TaskList list)
{
	IPC_Task task = NULL;
	IPC_Task next = NULL;

	for(task = list->mHead; task; task = next) 
	{
		next = task->mNext;
		IPC_Free(task);
		list->mCnt--;
	}
}

int
IPC_Task_Loop()
{
	int nRet = 0;
	int nCnt = 0;
	int nLastFd = -1;
	IPC_Task task = NULL;
	
	struct timeval sWait;

	fd_set sReadFds;
	FD_ZERO(&sReadFds);

	for(task = _taskList.mHead; task; task = task->mNext)
	{
		nLastFd = MAX(task->mFd, nLastFd);
		FD_SET(task->mFd, &sReadFds);
	}

	if(nLastFd < 0)
		goto EXIT;

	nLastFd++;
	
	sWait.tv_sec = 2;
	sWait.tv_usec = 0;

	nCnt = select(nLastFd, &sReadFds, NULL, NULL, &sWait);

	if(nCnt < 1)
	{
		if(nCnt == -1 && errno != EINTR)
			IPC_LogE("select error oucced!!! errno=%d\n", errno);
		
		goto EXIT;	
	}
	
	for(task = _taskList.mHead; task; task = task->mNext) 
	{
		if(! task || task->mFd < 0)
			continue;

		if(FD_ISSET(task->mFd, &sReadFds) && task->mProc)
		{
			if(task->mProc(task, task->mParam) < 0)
			{
				IPC_LogE("Thread proc failed !!! auto remove Thread(%s) !!\n", task->mName);

				_ipc_task_list_delete(&_taskList, task);
			}
		}
	}

EXIT:
	return nRet;
}


IPC_Task 
IPC_Task_Create(const char* pszName, int nFd, IPC_Task_Proc fnProc, void* pParam)
{
	IPC_Task task = (IPC_Task)IPC_Malloc(sizeof(struct IPC_Task_s));

	if(!task || !pszName || nFd < 0)
		return NULL;

	task->mName  = pszName;
	task->mProc  = fnProc;
	task->mFd    = nFd;
	task->mParam = pParam;

	return task;
}

int 
IPC_Task_Start(IPC_Task task)
{
	if(task == NULL)
		return -1;

	IPC_LogI("Start thread(%s) ....\n", task->mName);

	_ipc_task_list_add(&_taskList, task);

	return 0;
}


int 
IPC_Task_Cancel(IPC_Task task)
{
	if(task == NULL)
	{
		IPC_LogE("Invalid Thread Inputed !!!\n");
		return -1;
	}

	IPC_LogI("Cancel thread(%s) ....\n", task->mName);

	_ipc_task_list_delete(&_taskList, task);

	return 0;
}


void 
IPC_Task_Destroy(IPC_Task task)
{
	_ipc_task_list_delete_all(&_taskList);

	if(task)
		IPC_Free(task);
}

