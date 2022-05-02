#ifndef __IPC_TASK_H_
#define __IPC_TASK_H_

#include "ipc.h"

typedef struct IPC_Task_s* IPC_Task;

typedef int (*IPC_Task_Proc)(IPC_Task task, void* param);

IPC_Task IPC_Task_Create(const char* pszName, int nFd, IPC_Task_Proc fnProc, void* pParam);
int      IPC_Task_Start(IPC_Task task);
int      IPC_Task_Stop(IPC_Task task);
void     IPC_Task_Destroy(IPC_Task task);
int      IPC_Task_Loop(void);

#endif // __IPC_TASK_H_
