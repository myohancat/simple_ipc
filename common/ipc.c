/*
 * Simple IPC Source code
 * 
 * AUTHOR : kykim 
 */

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

#define DEFAULT_POLL_TIMEOUT   	2500    /* 2.5 sec */
#define INFINITE_POLL_TIMEOUT   -1

#define IPC_STARTBITS 0xB175

typedef struct IPC_Header_s
{
	UINT16 mStartBits;
	UINT16 mCmd;
	UINT32 mLen; // Except Header
} IPC_Header_t;

static int
poll_wrapper(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int nRet = 0;
	sigset_t sOrig;
	sigset_t sMask;

	memset(&sOrig, 0, sizeof(sOrig));
	memset(&sMask, 0, sizeof(sMask));

	sigaddset(&sMask, SIGHUP);
	sigaddset(&sMask, SIGALRM);
	sigaddset(&sMask, SIGUSR1);
	sigaddset(&sMask, SIGUSR2);

	sigprocmask(SIG_SETMASK, &sMask, &sOrig);
	nRet = poll(fds, nfds, timeout);
	sigprocmask(SIG_SETMASK, &sOrig, NULL);

	return nRet;
}

static int
fd_poll(int nFd, int nReq, int timeout)
{
	struct pollfd sFd;
	int nPoll = 0;

	sFd.fd = nFd;
	sFd.events = (short)nReq;
	sFd.revents = 0;

	nPoll = poll_wrapper(&sFd, 1, timeout);

	if ( nPoll == 0 ) 
	{
		IPC_LogE("poll timeout - %d msec. GIVE UP.\n", timeout);
		return 1;	/* poll timeout */
	}

	if ( nPoll < 0 ) 
	{
		IPC_LogE("poll failed. ret=%d, errno=%d.\n", nPoll, errno);
		return -errno;	/* poll failed */
	}

	if ( sFd.revents & (POLLRDHUP | POLLERR | POLLHUP | POLLNVAL) ) {
		if ( sFd.revents & POLLRDHUP )
			IPC_LogE("POLLRDHUP.\n");
		if ( sFd.revents & POLLERR )
			IPC_LogE("POLLERR.\n");
		if ( sFd.revents & POLLHUP )
			IPC_LogE("POLLHUP.\n");
		if ( sFd.revents & POLLNVAL )
			IPC_LogE("POLLNVAL.\n");

		return -1;	/* fd error detected */
	}

	return 0;
}

static int
_msg_read(int nFd, void* pBuf, int nBufSize)
{
	int nRet = 0;
	int nRead = 0;
	int nTotalRead = 0;

	while( nTotalRead < nBufSize )
	{
		nRet = fd_poll(nFd, POLLIN, INFINITE_POLL_TIMEOUT);
		if( nRet )
		{
			IPC_LogE("error poll. may be socket closed \n");
			return nRet;	
		}
		
		nRead = read(nFd, (void*)((UINT8*)pBuf + nTotalRead), nBufSize - nTotalRead);
		if(nRead > 0)
		{
			nTotalRead += nRead;
			continue;
		}
		
		return -1;
	}

	return 0;
}

static int
_msg_write(int nFd, void* pBuf, int nBufSize)
{
	int nRet;
	int nWrite = 0;
	int nTotalWrite = 0;

	while( nTotalWrite < nBufSize)
	{
		nRet = fd_poll(nFd, POLLOUT, DEFAULT_POLL_TIMEOUT);
		if( nRet )
		{
			IPC_LogE("error poll. may be socket closed \n");
			return nRet;	
		}
		
		nWrite = write(nFd, ((UINT8*)pBuf + nTotalWrite), nBufSize - nTotalWrite);
		if(nWrite > 0)
		{
			nTotalWrite += nWrite;
			continue;
		}

		return -1;
	}

	return 0;
}

int IPC_Session_Send(IPC_Session session, int nCmd, IPC_Parcel pParcel)
{
	IPC_Header_t sIpcHdr;

	if(session == NULL || session->mFd < 0)
	{
		IPC_LogE("Invalid Client Inputed !!!\n");
		return -1;
	}
	
	pParcel->Rewind(pParcel);

	sIpcHdr.mStartBits = IPC_STARTBITS;
	sIpcHdr.mCmd       = nCmd;
	sIpcHdr.mLen       = pParcel->mDataSize;
	
	if(_msg_write(session->mFd, &sIpcHdr, sizeof(sIpcHdr)) < 0)
	{
		IPC_LogE("[Session : %s] Cannot write ipc header. maybe connection is closed !\n", session->mURI);
		return -1;
	}
	if(_msg_write(session->mFd, pParcel->mData, pParcel->mDataSize) < 0)
	{
		IPC_LogE("[Session : %s] Cannot write ipc body. maybe connection is closed !\n", session->mURI);
		return -1;
	}
	return 0;
}

int IPC_Session_Recv(IPC_Session session, int* pCmd, IPC_Parcel pParcel)
{
	IPC_Header_t sIpcHdr;

	if(session == NULL || session->mFd < 0)
	{
		//IPC_LogE("Invalid Client Inputed !!!\n");
		return -1;
	}
	
	pParcel->Reset(pParcel);

	if(_msg_read(session->mFd, &sIpcHdr, sizeof(sIpcHdr)) < 0)
	{
		IPC_LogE("[Session : %s] Cannot read ipc header. maybe connection is closed !\n", session->mURI);
		return -1;
	}

	if(sIpcHdr.mLen > 0)
	{
		UINT8* pBuf = (UINT8*)IPC_Malloc(sIpcHdr.mLen);

		if(_msg_read(session->mFd, pBuf, sIpcHdr.mLen) < 0)
		{
			IPC_LogE("[Session : %s] Cannot read ipc body. maybe connection is closed !\n", session->mURI);
			return -2;
		}
		pParcel->mData = pBuf;
		pParcel->mDataPos = 0;
		pParcel->mDataSize = sIpcHdr.mLen;
		pParcel->mDataCapacity = sIpcHdr.mLen;
	}

	*pCmd = sIpcHdr.mCmd;

	return 0;
}

IPC_Session IPC_Session_Create(int nFd, const char* pszURI)
{
	IPC_Session session = (IPC_Session)IPC_Malloc(sizeof(struct IPC_Session_s));
	if(session == NULL)
		return NULL;	

	session->mFd = nFd;
	session->mURI = pszURI;

	session->Send = IPC_Session_Send;
	session->Recv = IPC_Session_Recv;

	return session;
}

void IPC_Session_Destroy(IPC_Session session)
{
	if(session->mFd >= 0)
		close(session->mFd);

	IPC_Free(session);
}
