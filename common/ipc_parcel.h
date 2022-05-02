#ifndef __IPC_PARCEL_H_
#define __IPC_PARCEL_H_

IPC_Parcel  IPC_Parcel_Create(void);
void        IPC_Parcel_Destroy(IPC_Parcel parcel);

void        IPC_Parcel_Rewind(IPC_Parcel parcel);
void        IPC_Parcel_Reset(IPC_Parcel parcel);

int         IPC_Parcel_Write8(IPC_Parcel parcel,  UINT8  val);
int         IPC_Parcel_Write16(IPC_Parcel parcel, UINT16 val);
int         IPC_Parcel_Write32(IPC_Parcel parcel, UINT32 val);
int         IPC_Parcel_WriteString(IPC_Parcel parcel, const char* str);
int         IPC_Parcel_WriteBytes(IPC_Parcel parcel, const void* data, int len);

int         IPC_Parcel_Read8(IPC_Parcel parcel, UINT8* pVal);
int         IPC_Parcel_Read16(IPC_Parcel parcel, UINT16* pVal);
int         IPC_Parcel_Read32(IPC_Parcel parcel, UINT32* pVal);
const char* IPC_Parcel_ReadString(IPC_Parcel parcel);
UINT8*      IPC_Parcel_ReadBytes(IPC_Parcel parcel, void* data, int len);

#endif /* __IPC_PARCEL_H_ */
