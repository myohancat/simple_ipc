#include "ipc.h"

#include "ipc_def.h"
#include "ipc_task.h"
#include "ipc_stub.h"

#include <stdio.h>

int _ipc_stub_proc(int cmd, IPC_Parcel data, IPC_Parcel reply, void* param)
{
    switch(cmd)
    {
        case IPC_SAMPLE_CMD_ECHO:
        {
            const char* str = data->ReadString(data);

            // Function Call
            printf("%s recved !!\n", str);
            
            reply->WriteString(reply, str);
            break;    
        }
        default:
        {
            printf("Unkown Cmd : %d Inputed !\n", cmd);
        }        
    }

    return 0;
}

int _ipc_server_task_proc(IPC_Task task, void* param)
{
    IPC_Server  server = (IPC_Server)param;
    IPC_Session session = IPC_Server_Accept(server);

    IPC_Stub_Create(session, _ipc_stub_proc, NULL);

    return 0;
}

int main(int argc, char* argv[])
{
    IPC_Server server = IPC_Server_Create("/tmp/ipc");

    IPC_Task task = IPC_Task_Create("ipc", server->mFd, _ipc_server_task_proc, server);

    IPC_Task_Start(task);

    while(1)
    {
        IPC_Task_Loop();
    }

    return 0;
}
