#include "ipc.h"

#include "ipc_def.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
	int cmd = 0;
	char buff[1024] = { 0, };
	char* p;
	const char* echo = NULL;

	IPC_Parcel data = IPC_Parcel_Create();
	IPC_Parcel reply = IPC_Parcel_Create();

	IPC_Session session = IPC_Client_Connect("/tmp/ipc");
	if(session == NULL)
	{
		printf("Connection failed !\n");
		return -1;
	}

	while(1)
	{
		cmd = IPC_SAMPLE_CMD_ECHO;

		fgets(buff, sizeof(buff) -1, stdin);
		if( (p = strchr(buff, '\n')) != NULL) *p = '\0';

		data->WriteString(data, buff);
			
		if(session->Send(session, cmd, data) != 0)
			break;

		if(session->Recv(session, &cmd, reply) != 0)
			break;

		echo = reply->ReadString(reply);

		printf("%s\n", echo);

		data->Reset(data);
		reply->Reset(reply);
	}

	if(session != NULL)
		IPC_Session_Destroy(session);

	IPC_Parcel_Destroy(data);
	IPC_Parcel_Destroy(reply);

	return 0;
}
