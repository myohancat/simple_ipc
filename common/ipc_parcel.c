#include "ipc.h"

#define DEFAULT_PARCEL_DATA_SIZE     (256)

static int _GrowData(IPC_Parcel parcel, UINT32 size)
{
    UINT32 newSize = ((parcel->mDataCapacity+size)*3)/2; 

#if 1 // Enhance Performance 
    if(newSize < DEFAULT_PARCEL_DATA_SIZE)
        newSize = DEFAULT_PARCEL_DATA_SIZE;
#endif 

    if(parcel->mData == NULL)
        parcel->mData = (UINT8*)IPC_Malloc(newSize);
    else
        parcel->mData = (UINT8*)IPC_Realloc(parcel->mData, newSize);

    if(parcel->mData == NULL)
    {
        parcel->mDataCapacity = 0;
        return -1;
    }

    parcel->mDataCapacity = newSize;
    return 0;
}

IPC_Parcel IPC_Parcel_Create(void)
{
    IPC_Parcel parcel = (IPC_Parcel)IPC_Malloc(sizeof(struct IPC_Parcel_s));

    if(parcel == NULL)
        return NULL;

    memset(parcel, 0x00, sizeof(struct IPC_Parcel_s));

    return parcel;
}

void IPC_Parcel_Destroy(IPC_Parcel parcel)
{
    if(parcel == NULL)
        return;

    if(parcel->mData != NULL)
        IPC_Free(parcel->mData);
    
    IPC_Free(parcel);
}

void IPC_Parcel_Rewind(IPC_Parcel parcel)
{
    parcel->mDataPos = 0;
}

void IPC_Parcel_Reset(IPC_Parcel parcel)
{
    parcel->mDataPos  = 0;
    parcel->mDataSize = 0;
    parcel->mDataCapacity = 0;

    IPC_SAFE_FREE(parcel->mData);
}

int IPC_Parcel_Write8(IPC_Parcel parcel,  UINT8  val)
{

}

int IPC_Parcel_Write16(IPC_Parcel parcel, UINT16 val)
{

}

int IPC_Parcel_Write32(IPC_Parcel parcel, UINT32 val)
{

}

int IPC_Parcel_WriteString(IPC_Parcel parcel, const char* str)
{
}

int IPC_Parcel_WriteBytes(IPC_Parcel parcel, const void* data, int len)
{
}


int IPC_Parcel_Read8(IPC_Parcel parcel, UINT8* pVal)
{
}

int IPC_Parcel_Read16(IPC_Parcel parcel, UINT16* pVal)
{
}

int IPC_Parcel_Read32(IPC_Parcel parcel, UINT32* pVal)
{
}

const char* IPC_Parcel_ReadString(IPC_Parcel parcel)
{

}

UINT8* IPC_Parcel_ReadBytes(IPC_Parcel parcel, void* data, int len)
{

}

