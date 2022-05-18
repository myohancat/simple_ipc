#ifndef __IPC_H_
#define __IPC_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Type Definition
 */
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef char            CHAR;

#define IPC_Malloc        malloc
#define IPC_Realloc     realloc
#define IPC_Calloc        calloc
#define IPC_Strdup         strdup
#define IPC_Free         free

#define IPC_SAFE_FREE(x)     if((x) != NULL) { IPC_Free(x); (x) = NULL; }

/*
 * Parcel Definition
 */
typedef struct IPC_Parcel_s* IPC_Parcel;

extern IPC_Parcel IPC_Parcel_Create(void);
extern void       IPC_Parcel_Destroy(IPC_Parcel parcel);

/*
 * Session Definition
 */
typedef struct IPC_Session_s* IPC_Session;

struct IPC_Session_s
{
    int         mFd;
    const char* mURI;
    
    int (*Send)(IPC_Session conn, int cmd, IPC_Parcel parcel);
    int (*Recv)(IPC_Session conn, int* pCmd, IPC_Parcel parcel);
};

extern IPC_Session IPC_Session_Create(int nFd, const char* pszURI);
extern void        IPC_Session_Destroy(IPC_Session conn);

/*
 * Server Definition
 */
#define MAX_URI_LEN  (1024)
typedef struct IPC_Server_s* IPC_Server;

struct IPC_Server_s
{
    char mURI[MAX_URI_LEN];
    int  mFd;
};

extern IPC_Server  IPC_Server_Create(const char* pszURI);
extern IPC_Session IPC_Server_Accept(IPC_Server server);
extern void        IPC_Server_Destroy(IPC_Server server);


/*
 * Client Definition
 */
extern IPC_Session IPC_Client_Connect(const char* pszURI);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IPC_H_ */
