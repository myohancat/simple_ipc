#include "ipc_stub.h"

#include "ipc_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define MAX(x, y) x>y?x:y

static void* _IPC_Stub_Task_Proc(void *pParam)
{
	IPC_Stub    stub    = (IPC_Stub)pParam;
	IPC_Session session = NULL;

	int nCnt = 0;
	int nLastFd = 0;
	struct timeval sWait;
	fd_set sReadFds;

	IPC_Parcel data = IPC_Parcel_Create();
	IPC_Parcel reply = IPC_Parcel_Create();
 
	if(stub == NULL)
		return NULL;

	session = stub->mSession;
	nLastFd = MAX(stub->mPipe[0], session->mFd);

	while(1)
	{
		FD_ZERO(&sReadFds);
		FD_SET(stub->mPipe[0], &sReadFds);
		FD_SET(session->mFd, &sReadFds);

		sWait.tv_sec = 1;
		sWait.tv_usec = 0; //100000; /* 0.1 sec */

		nCnt = select(nLastFd, &sReadFds, NULL, NULL, &sWait);
		if(nCnt < 1)
		{
			if(nCnt == -1 && errno != EINTR)
			{
				IPC_LogE("select error oucced!!! errno=%d\n", errno);
				goto EXIT;	
			}

			continue; // Skip for Timeout
		}
	
		if(FD_ISSET(stub->mPipe[0], &sReadFds))
			goto EXIT;

		if(FD_ISSET(session->mFd, &sReadFds))
		{
			int nCmd;

			if(session->Recv(session, &nCmd, data) < 0)
				goto EXIT;

			reply->Reset(reply);

			if(stub->mProc(nCmd, data, reply, stub->mProcParam) < 0)
				goto EXIT;

			if(session->Send(session, nCmd, reply) < 0)
				goto EXIT;
		}
	}

EXIT:
	IPC_Parcel_Destroy(data);
	IPC_Parcel_Destroy(reply);

	return NULL;
}


IPC_Stub IPC_Stub_Create(IPC_Session session, IPC_Stub_Proc_fn fnProc, void* procParam)
{
	IPC_Stub stub = NULL;

	stub = (IPC_Stub)IPC_Malloc(sizeof(struct IPC_Stub_s));
	memset(stub, 0x00, sizeof(struct IPC_Stub_s));

	if(pipe(stub->mPipe) < 0)
	{
		IPC_LogE("create pipe failed !\n");
		goto EXIT;
	}

	stub->mSession    = session;
	stub->mProc       = fnProc;
	stub->mProcParam  = procParam;

	if(pthread_create(&stub->mThread, NULL, _IPC_Stub_Task_Proc, stub) < 0)
    {
		IPC_Free(stub);
	}
EXIT:	
	return stub;
}

void IPC_Stub_Stop(IPC_Stub stub)
{
	void* pRet;
	write(stub->mPipe[1], "S", 1);

	pthread_join(stub->mThread, &pRet);
}

void IPC_Stub_Destroy(IPC_Stub stub)
{
	if(stub == NULL)
		return;

	IPC_Free(stub);
}
