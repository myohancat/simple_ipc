#ifndef __IPC_LOG_H_
#define __IPC_LOG_H_

#define MODULE_HDR "[IPC]"

#define IPC_LogE(...)  \
		do { \
				printf(MODULE_HDR " (Err) %s():%d ", __FUNCTION__, __LINE__); \
				printf(__VA_ARGS__); \
		} while(0)

#define IPC_LogI(...)  \
		do { \
				printf(MODULE_HDR " %s():%d ", __FUNCTION__, __LINE__); \
				printf(__VA_ARGS__); \
		} while(0)

#define IPC_LogD(...)  \
		do { \
				printf(__VA_ARGS__); \
		} while(0) 

#define IPC_LogD1(...)  \
		do { \
				printf(MODULE_HDR fmt, __VA_ARGS__); \
		} while(0) 


#endif /* __IPC_LOG_H_ */
