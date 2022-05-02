#include "ipc.h"

#include "ipc_log.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>

IPC_Session IPC_Client_Connect(const char* pszURI)
{
	int nRet;
	struct sockaddr_un sServerAddr;
	IPC_Session pConnect = NULL;

	int nFd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if(nFd < 0)
	{
		IPC_LogE("socket() failed. errno=%d.\n", errno);
		goto ERROR;
	}
	
	memset(&sServerAddr, 0x00, sizeof(sServerAddr));
	sServerAddr.sun_family = AF_LOCAL;
	strcpy(sServerAddr.sun_path, pszURI);

	nRet = connect(nFd, (struct sockaddr*)&sServerAddr, sizeof(sServerAddr));
	if(nRet)
	{
		IPC_LogE("connect failed. errno=%d.\n", errno);
		goto ERROR;	
	}

	pConnect = IPC_Session_Create(nFd, pszURI);
	if(pConnect == NULL)
		goto ERROR;
	
	goto EXIT;
ERROR:
	if(nFd >=0)
		close(nFd);
	
EXIT:
	return pConnect;
}

