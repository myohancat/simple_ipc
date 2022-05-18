#include "ipc.h"

#include "ipc_log.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ST_SERVER_BACKLOG    5

int exists(const char* filename)
{
    if(access(filename, F_OK ) != -1) return 1;
    
    return 0;    
}

IPC_Server  IPC_Server_Create(const char* pszURI)
{
    IPC_Server server = NULL;
    int nRet = 0;
    int nFd  = -1;

#if 1 /* W/A auto remove previous unix domain socket file */
    if(exists(pszURI))
    {
        //IPC_LogI("File is already exist : %s, remove it !!\n", pszURI);
        unlink(pszURI);
    }
#endif
    
    /* create socket */
    nFd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (nFd < 0) 
    {
        IPC_LogE("socket() failed. errno=%d.\n", errno);
        goto ERROR;
    }

    /* set socket option: reuse address */
    {
        int nOn = 1;

        nRet = setsockopt(nFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOn, sizeof(nOn));
        if(nRet) 
        {
            IPC_LogE("setsockopt(SO_REUSEADDR) failed. errno=%d.\n", errno);
            goto ERROR;
        }
    }

    /* set socket option: linger, make sure all data xmitted when close */
    {
        struct linger sLinger;
        sLinger.l_onoff = 1;
        sLinger.l_linger = 30;

        nRet = setsockopt(nFd, SOL_SOCKET, SO_LINGER, (const char*)&sLinger, sizeof(sLinger));
        if(nRet) 
        {
            IPC_LogE("setsockopt(SO_LINGER) failed. errno=%d.\n", errno);
            goto ERROR;
        }
    }

    /* bind socket */
    {
        struct sockaddr_un sSvcAddr;
        sSvcAddr.sun_family = AF_LOCAL;
        strcpy(sSvcAddr.sun_path, pszURI);

        nRet = bind(nFd, (struct sockaddr*)&sSvcAddr, sizeof(struct sockaddr_un));

        if(nRet && errno == EADDRINUSE) 
        {
            IPC_LogE("socket is already in use.\n");
            goto ERROR;
        } 
        else if(nRet) 
        {
            IPC_LogE("bind() failed. errno=%d.\n",errno);
            goto ERROR;
        }
    }

    /* listen socket */
    {
        nRet = listen(nFd, ST_SERVER_BACKLOG);
        if(nRet) 
        {
            IPC_LogE("%s(): listen() failed. errno=%d.\n", __FUNCTION__, errno);
            goto EXIT;
        }
    }

    server = (IPC_Server)IPC_Malloc(sizeof(struct IPC_Server_s));
    if(server == NULL)
    {
        IPC_LogE("Cannot alloc IPC Server Handle\n");
        goto ERROR;
    }

    memset(server, 0x00, sizeof(struct IPC_Server_s));
    server->mFd = nFd;
    strncpy(server->mURI, pszURI, MAX_URI_LEN - 1);
    server->mURI[MAX_URI_LEN - 1] = '\0';

    goto EXIT;
ERROR:
    if(nFd >= 0)
    {
        close(nFd);
        nFd = -1;
    }    

    if(server != NULL)
    {
        IPC_Free(server);
        server = NULL;
    }

EXIT:

    return server;
}

IPC_Session IPC_Server_Accept(IPC_Server server)
{
    IPC_Session session = NULL;

    /* accept socket */
    {
        struct sockaddr_un sClntAddr;
        int nClntFd = -1;
           unsigned int nClntAddrLen = sizeof(sClntAddr);

        IPC_LogD("%s(): accepting new connection request.\n", __FUNCTION__);
        memset(&sClntAddr, 0, nClntAddrLen);


        nClntFd = accept(server->mFd, (struct sockaddr*)&sClntAddr, &nClntAddrLen);
           if ( nClntFd < 0 ) 
        {
               IPC_LogE("accept() failed. errno=%d.\n", errno);
               goto EXIT;
        }

        session = IPC_Session_Create(nClntFd, server->mURI);
        if(session == NULL)
        {
            close(nClntFd);
            goto EXIT;
        }
    }

EXIT:
    return session;
}

void IPC_Server_Destroy(IPC_Server server)
{
    if(server == NULL)
        return;

    if(server->mFd != -1)
        close(server->mFd);

    unlink(server->mURI);
    IPC_Free(server);
}

