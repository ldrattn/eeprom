#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ldrattnapp.h"
#include "debug.h"
#include "logger.h"
#include "cfgparser.h"
#include "csvparser.h"
#include "actiondef.h"

static void printCalibData();
static void readLDRConfig();
static int cmdParser(char *clientcmd,int sockfd);
static void startCalibrationThread(unsigned int ,unsigned int,unsigned int);
static void stopCorrectionThread(void);
static void startCorrectionThread(void);
static void stopCorrectionThread(void);

#define SOCK_PATH "/usr/local/etc/ldrattn/echo_socket"
#define CMDLEN 32
#define MAX_ARG 7
#define DEBUG 1

LDRAttr ldrattn;
LDRConf ldrconf;
int eepromfd;

unsigned int calibStop;
unsigned int deltaStop;
unsigned int crnState;

unsigned int curVolume;
unsigned int curImpedance; //curImpedance will be changed based on the callibrartion
			    //customer may do calibrartion for different impedance 	

struct volumeattr attr;
struct calibattr calibdata;
int pipefd[2];
char header[HEADER_LEN];



Calibstate State;
	
pthread_t deltaThread;
pthread_t calibThread;
void deltaCorrection(struct volumeattr *);
int doCalibration(struct calibattr *);

static void printCalibData()
{

#ifdef DEBUG
	int db;
	for(db =0 ;db < 22;db++) {
		printf(" %d  %d  %d  %d  %d  %d  %d  %d  %d %d \n",ldrattn.dataR[db].pw_SE,ldrattn.dataR[db].pw_SH,ldrattn.dataR[db].i_SE,ldrattn.dataR[db].i_SH,ldrattn.dataL[db].pw_SE,ldrattn.dataL[db].pw_SH,ldrattn.dataL[db].i_SE,ldrattn.dataL[db].i_SH,ldrattn.targetres[db].series, ldrattn.targetres[db].shunt);
		printf("\n");		
	
	}
#endif	

}

static void readLDRConfig()
{
	char csvfile[MAXLEN];

	loadDefParams();		
	if(readParseCfg() != EXIT_SUCCESS){
		debugLog(LOG_INFO, "Failed to Read Config file,so loading default values\n");
	}
	LOG_TRACE(LOG_INFO,"the config values %d %d %d\n",ldrconf.volume,ldrconf.impedance,ldrconf.calibSteps);		 			
	curVolume = ldrconf.volume;
	curImpedance = ldrconf.impedance;

	memset(csvfile,'\0',MAXLEN);
	getCSVFilename(ldrconf.impedance,csvfile);

	if(readCalibData(csvfile) == EXIT_SUCCESS) {
		
		printCalibData();
		setVolume(curVolume);
		startCorrectionThread();		
		State = CALIBRATED;
		LOG_TRACE(LOG_INFO,"Read Calibrated Data \n");		 			
	} else {
		LOG_TRACE(LOG_INFO,"Failed to Read Calibrated Data Maybe calibrration not done\n");
		//we are check for file size null. csv format cheking should be done on GUI side while
		//uploading .
		debugLog(LOG_INFO, "Failed to Read Calibrated Data Maybe calibrration not done\n");
		setMute();
		State = NOTCALIBRATED;
	}
}

static void startCorrectionThread()
{
	int ret;

	attr.volume = curVolume;
	if(crnState == CRN_NONE) {
		ret  = pthread_create(&deltaThread,NULL,(void *)deltaCorrection,(void *)&attr);
		if(ret == 0){
			LOG_TRACE(LOG_INFO,"Created Delta correction thread\n");
			crnState = CRN_ONGOING; 
		} else {
			LOG_TRACE(LOG_INFO,"Failed to Create Delta correction thread\n");
			//todo what to do if it fails to create the thread
		}
	} else {
		LOG_TRACE(LOG_INFO,"Delta correction thread already running\n");

	}	
}

static void startCalibrationThread(unsigned int impedance,unsigned int numsteps,unsigned int temperature)
{
	int ret;

	memset(&calibdata,0,sizeof(calibdata));
	calibdata.impedance = impedance;
	calibdata.numsteps = numsteps;
	calibdata.temperature = temperature;
	calibdata.status = CALIB_NONE;
	calibdata.pipefd = pipefd[1];
	calibdata.shstep = 0;
	calibdata.sestep = 0;

	ret = pthread_create(&calibThread,NULL,(void *)doCalibration,(void *)&calibdata);

	if(ret == 0) {
		LOG_TRACE(LOG_INFO,"Calibrartion thread created\n");

	} else {
		LOG_TRACE(LOG_INFO,"Failed  to create the calibration thread \n");
		calibdata.status = CALIB_FAILED;
		//todo what to do if it fails to create the thread
	}
}

static void stopCorrectionThread()
{
#if 0
	int ret;

	ret = pthread_cancel(deltaThread);
 	if(ret) {
		LOG_TRACE(LOG_INFO,"Failed to Cancel Delta correction thread %d\n",ret);

	}
#endif

	deltaStop = TRUE;
	pthread_join(deltaThread,NULL);
	crnState = CRN_NONE; 
	printf("stopCorrectionThread ---  \n");		
}

static void stopCalibrationThread()
{
#if 0
	int ret;

	ret = pthread_cancel(calibThread);
 	if(ret) {
		LOG_TRACE(LOG_INFO,"Failed to Cancel Calibration thread %d\n",ret);
	}
#endif

	calibStop = TRUE;
	pthread_join(calibThread,NULL);	
	printf("stopCalibrationThread ---  \n");		

}

int getTemperature()
{

	return 25;

} 
static int cmdParser(char *clientcmd,int sockfd)
{
	unsigned int argvalue;	
	unsigned int calibsteps;
	unsigned int impedance = 0;	
	unsigned int temperature;	
	char respbuf[MAXLEN];

	if(!strlen(clientcmd))
		return EXIT_FAILURE;
	char *argc[MAX_ARG];
	int argcnt = 0;
	argc[argcnt] = strtok(clientcmd," ");
	while((argc[argcnt++] != NULL) && (argcnt < MAX_ARG)) {
		
		argc[argcnt] = strtok(NULL," ");
	}

	argcnt = 0;
	if(!strncmp(argc[argcnt],"CALIB_START",strlen(argc[argcnt]))) { 
	
		if((calibdata.status == CALIB_ONGOING)) {
			LOG_TRACE(LOG_INFO,"calibration is ongoing,Calibration request is ignored \n");
		} else {
			//Todo call stop calibration
			//stop thread
			stopCorrectionThread();
			//do calibration

			impedance = atoi(argc[++argcnt]);
			calibsteps = atoi(argc[++argcnt]);
			temperature = getTemperature(); 
#if 0
			ldrconf.calibSteps = calibsteps;
#endif
			printf("----------------- impedance %d\n",impedance);
			//Todo de we need to set the volume to default volume or current volume after calibration 			
			if(calibsteps <= CALIB_MAXSTEP) {	
				startCalibrationThread(impedance,calibsteps,temperature);
			} else {
				LOG_TRACE(LOG_INFO,"Calibrartion step exceeded Maximum limit \n");
				debugLog(LOG_INFO, "Calibrartion step exceeded Maximum limit \n");

			}
		}
 
	} else if(!strncmp(argc[argcnt],"CALIB_STOP",strlen(argc[argcnt]))) {
		//Todo how to stop calibration
		memset(&calibdata,0,sizeof(calibdata));
		stopCalibrationThread();
		startCorrectionThread();
		printf("cALIBRATION STOPPED ------------------\n");	
	
	} else if(!strncmp(argc[argcnt],"SET_VOLUME",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		stopCorrectionThread();
		argvalue = atoi(argc[++argcnt]);
		attr.balanceChan = 0;
		attr.balanceValue = 0;
		setVolume(argvalue);
		curVolume = argvalue;
		if((curVolume != 0) &&  (curVolume < (ldrconf.calibSteps - 1))) {
			startCorrectionThread();		
		}
		//set volume
	} else if(!strncmp(argc[argcnt],"RELOAD_CONFIG",strlen(argc[argcnt]))) {
		memset(&ldrattn,0,sizeof(ldrattn));
		stopCorrectionThread();
		setMute();
		readLDRConfig();
		State = NOTCALIBRATED;	
		//reload config

	} else if(!strncmp(argc[argcnt],"CALIB_SAVE",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		if(saveCalibration() == EXIT_FAILURE) {
			debugLog(LOG_INFO, "Failed to Save Calibrated Data \n");
		} else {  
			memset(&calibdata,0,sizeof(calibdata));
		}		 
		//save calibration

	} else if(!strncmp(argc[argcnt],"WRITE_CONFIG",strlen(argc[argcnt]))) {
		//write to config file
		writeLDRConfig(argc);
		stopCorrectionThread();
		readLDRConfig();
		State = NOTCALIBRATED;	

	} else if(!strncmp(argc[argcnt],"BALANCE_UPDATE",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		attr.balanceChan = atoi(argc[++argcnt]);
		attr.balanceValue = atoi(argc[++argcnt]);
		stopCorrectionThread();
		startCorrectionThread();		
		
	
	} else if(!strncmp(argc[argcnt],"TEMP_COMPENSATE",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		//Todo do the DAC correction based on the previous values
		argvalue = atoi(argc[++argcnt]);
		 
	} else if(!strncmp(argc[argcnt],"MUTE_VOLUME",strlen(argc[argcnt]))) {
		LOG_TRACE(LOG_INFO," Mute volume from client\n");		 			
		stopCorrectionThread();
		setMute();
		 
	} else if(!strncmp(argc[argcnt],"CALIB_STATUS",strlen(argc[argcnt]))) {
		LOG_TRACE(LOG_INFO,"Calib Status from client\n");
		snprintf(respbuf,MAXLEN,"%d,%d,%d,%d,%d",calibdata.impedance,calibdata.numsteps,calibdata.status,calibdata.sestep,calibdata.shstep);
		printf("calibstatus is %s\n",respbuf);
		send(sockfd,respbuf,strlen(respbuf),0);		 			
		 
	} else if(!strncmp(argc[argcnt],"INC_VOLUME",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		stopCorrectionThread();
		attr.balanceChan = 0;
		attr.balanceValue = 0;
		if(curVolume < (ldrconf.calibSteps - 1)) {
			setVolume(++curVolume);
		}
		if((curVolume > 0) &&  (curVolume < (ldrconf.calibSteps - 1))) {
			startCorrectionThread();		
		}
	} else if(!strncmp(argc[argcnt],"DEC_VOLUME",strlen(argc[argcnt])) && (State == CALIBRATED)) {
		stopCorrectionThread();
		attr.balanceChan = 0;
		attr.balanceValue = 0;
		if(curVolume > 0) {
			setVolume(--curVolume);
		}	
		if((curVolume > 0) &&  (curVolume < (ldrconf.calibSteps - 1))) {
			startCorrectionThread();		
		}
	 
	} else {
		LOG_TRACE(LOG_INFO,"  Invalid command received from clieint\n");		 			
	}
	
	return EXIT_SUCCESS;
}


int main()
{
 
	unsigned int sock, ldrSock;
	struct sockaddr_un local, remote;
	socklen_t len;
	char cmd[CMDLEN];
	fd_set readfds;
	int maxfds;
	char ipcdata;
	unsigned int offset;
	unsigned short csvheader[HEADER_LEN];
	unsigned int ret;

	State = NOTCALIBRATED;

	openLog();	
	
	crnState = CRN_NONE;
#if 1	
	if(initEEPROM(eepromfd) < 0) {
		printf("Failed to initialize EEPROM\n");				

	}
	if( readHeaders(&csvheader,eepromfd) < 0 ) {
		LOG_TRACE(LOG_INFO,"  Failed to Read CSV Header \n");
		exit(1);
	} else {
		offset = 0x00;
		while( offset <= HEADER_LEN ) {
			if( getCSVFlag(offset ,&csvheader) == 0x1) {
				ret = rwEEPROMCSV(offset,&csvheader,eepromfd); 
				if( (ret < 0) || (ret == EXIT_FAILURE) ) {
					continue;
				}  	
			}		
			offset = offset+CSVHEADER_LEN;		
		}

	}
#endif
	
	if(initWiringPiDevices() != EXIT_SUCCESS){
		LOG_TRACE(LOG_INFO,"  Failed to initialise the I2C transaction \n");
		exit(1);
	}
	if(!testSetup()) {
		LOG_TRACE(LOG_INFO,"  Failed to initialise the ADC/DAC  transaction \n");
		exit(1);

	}
	readLDRConfig();
	if(pipe(pipefd) < 0) {
		LOG_TRACE(LOG_INFO,"  Failed to create pipe fd for IPC \n");
		exit(1);		
	}
			
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(sock, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock, 1) == -1) {
		perror("listen");
		exit(1);
	}
	
	for(;;) {
	
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		maxfds = sock;
	
		if(pipefd[0] > 0) {
			FD_SET(pipefd[0],&readfds);
		}
			
		if( pipefd[0] > maxfds) {
			maxfds = pipefd[0];	
		}
		
		printf("Waiting for a connection... %d %d %d\n",sock,pipefd[0],maxfds);
		select(maxfds+1, &readfds, NULL, NULL, NULL);	
		if(FD_ISSET(sock,&readfds)) {
			len = sizeof(remote);
			if ((ldrSock = accept(sock, (struct sockaddr *)&remote, &len)) == -1) {
				perror("accept");
				exit(1);
			}	
			recv(ldrSock, cmd, 80, 0);
			printf("the command received is %s \n",cmd);
			cmdParser(cmd,ldrSock);
			printf("Completed. %s\n",cmd);
			close(ldrSock);
		} 
		if(FD_ISSET(pipefd[0],&readfds)) {
			read(pipefd[0],&ipcdata,1);
			printf("starting correction thread \n");
			startCorrectionThread();		
		} 
		
	}

	close(ldrSock);
	close(sock);
	closeLog();
	return 0;
}

