#ifndef CFG_BUFFER
#define CFG_BUFFER

#define csize 100

typedef struct
{
	//---------- General Config
	char IP_SERVER[csize];
	long int TCP_PORT; 
	long int RCV_BUFFER_SIZE; 
	long int TX_DELAY; 

	char DIRECTORY_LOG[csize];

	//--------- DAEMON PID
	char SDTDPID[csize];

	//---------- GetPos Config - RingBuffer Shared Memory
	char GetPos_BackingFile[csize]; 
	char GetPos_SemaphoreName[csize]; 
	long int GetPos_AccessPerms; 
	
	long int RINGSIZE;
	long int WRITEBLK;
	long int GETPOS_INTERVAL;
	char DATAFILENAME[csize]; 
	char DATA_DIR[csize];

	//---------- Op_Mode Shared Memory Config 
	// - for start_obs, stop_obs, pTrack, scan and skydip
	char OpMode_BackingFile[csize]; 
	char OpMode_SemaphoreName[csize]; 
	long int OpMode_AccessPerms; 
	
} cfgBuffer_data;


#endif
