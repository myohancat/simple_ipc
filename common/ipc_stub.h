#ifndef __IPC_STUB_H_
#define __IPC_STUB_H_

#include "ipc.h"

typedef struct IPC_Stub_s* IPC_Stub;
typedef int (*IPC_Stub_Proc_fn)(int cmd, IPC_Parcel data, IPC_Parcel reply, void* param);

struct IPC_Stub_s
{
	pthread_t    mThread;

	int          mPipe[2];
	IPC_Session  mSession;
	
	IPC_Stub_Proc_fn mProc;
	void*            mProcParam;
};


IPC_Stub IPC_Stub_Create(IPC_Session session, IPC_Stub_Proc_fn fnProc, void* procParam);
void IPC_Stub_Stop(IPC_Stub stub);
void IPC_Stub_Destroy(IPC_Stub stub);

#endif // __IPC_STUB_H_
