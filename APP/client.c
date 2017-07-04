#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>
#include "common.h"


#define SOCK_PATH "/usr/local/etc/ldrattn/echo_socket"
#define MAXLEN 80
#define MINARGS 2 

struct ldrconf {
	unsigned int steps;
	unsigned int impedance;
	unsigned int balancechan;
	unsigned int balanceval;
	unsigned int volume;
	unsigned int temperature;
};


typedef enum { CALIB_START,SET_VOLUME,RELOAD_CONFIG,CALIB_SAVE,BALANCE_UPDATE,TEMP_COMPENSATE,WRITE_CONFIG,MUTE_VOLUME,CALIB_STATUS,CALIB_STOP,INC_VOLUME,DEC_VOLUME} LDRCmd; 


void print_usage()
{

printf("Usage: ./client\n\
        -C start calibrartion\n\
	-V set volume\n\
	-R reload configuration\n\
	-S save calibration\n\
	-T temperature compenstaion\n\
	-W write configuration\n\
	-B set balance update\n\
	-M Mute volume\n\
	-P Calib status\n\
	-O Calib stop\n\
	-I Increment volume\n\
	-D Decrement volume\n\
	-s arg number of calibrarion steps\n\
	-b arg balance value\n\
	-c arg balance channel\n\
	-i arg impedance\n\
	-v arg volume to be set\n\
	-t arg temperature to be set \n");

}

int main(int argc,char *argv[])
{
	int s, len;
	struct sockaddr_un remote;
	//char str[100];
	int option=0;
	int command;
	char cmd[MAXLEN];
	struct ldrconf conf;
	//LDRCmd ldrcmd;

	if(argc < MINARGS) {
		printf("please pass right arguments\n ");
		print_usage();
		return EXIT_FAILURE;
	}

	{
		conf.steps = DEFAULT_CALIB_STEPS;
		conf.impedance = DEFAULT_IMPEDANCE;
		conf.balancechan = LEFTCHAN;
		conf.balanceval = DEFAULT_BALANCE_VALUE_DB;
		conf.volume = DEFAULT_VOLUME;
		conf.temperature = DEFAULT_TEMPERATURE;
	}


	
	while((option = getopt(argc,argv,"CVRSTWBMPOIDs:b:c:i:v:t:")) != -1) {
		switch(option) {
			case 'C': //start calibrartion
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = CALIB_START;  	
				break;
			case 'V':  //set volume
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = SET_VOLUME;  	
				break;
			case 'R':  //reload config
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = RELOAD_CONFIG;  	
				break;
			case 'S':  //save calibration
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = CALIB_SAVE;  	
				break;
			case 'T':  //get temperature 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = TEMP_COMPENSATE;  	
				break;
			case 'W':  // write config
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = WRITE_CONFIG;  	
				break;
			case 'B':  // set balance 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = BALANCE_UPDATE;  	
				break;
			case 'M':  //Mute volume 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = MUTE_VOLUME;  	
				break;
			case 'P':  //calibration status 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = CALIB_STATUS;  	
				break;
			case 'O':  //calibration status 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = CALIB_STOP;  	
				break;
			case 'I':  //calibration status 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = INC_VOLUME;  	
				break;
			case 'D':  //calibration status 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				command = DEC_VOLUME;  	
				break;

			case 's': //number of steps
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.steps = atoi(optarg);		
				break;
			case 'b':  //balance value
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.balanceval = atoi(optarg);	
				break;
			case 'c':  //balance channel
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.balancechan = atoi(optarg); 
				break;
			case 'i':  //impedance
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.impedance = atoi(optarg); 
				break;
			case 'v':  //volume
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.volume = atoi(optarg); 
				break;
			case 't':  //temperature
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				conf.temperature = atoi(optarg); 
				break;
			default: 
				//printf("%s %s %d\n",__func__,__FILE__,__LINE__);		
				print_usage();
				exit(EXIT_FAILURE);

		}
	}
	
	memset(cmd,'\0',MAXLEN);
	//printf("commnad %d %d %d %d %d %d %d \n",command,conf.steps,conf.balancechan,conf.balanceval,conf.impedance,conf.volume,conf.temperature );	

	switch(command) 
	{
		case CALIB_START:
			snprintf(cmd,MAXLEN,"%s %d %d ","CALIB_START", conf.impedance,conf.steps);	
			break;
		case SET_VOLUME:
			snprintf(cmd,MAXLEN,"%s %d","SET_VOLUME", conf.volume);	
			break;
		case RELOAD_CONFIG:
			snprintf(cmd,MAXLEN,"%s","RELOAD_CONFIG");	
			break;
		case CALIB_SAVE:
			snprintf(cmd,MAXLEN,"%s","CALIB_SAVE");	
			break;
		case BALANCE_UPDATE:
			snprintf(cmd,MAXLEN,"%s %d %d","BALANCE_UPDATE",conf.balancechan,conf.balanceval );	
			break;
		case TEMP_COMPENSATE:
			snprintf(cmd,MAXLEN,"%s %d","TEMP_COMPENSATE",conf.temperature);	
			break;
		case WRITE_CONFIG:
			snprintf(cmd,MAXLEN,"%s %d %d %d %d %d %d","WRITE_CONFIG", conf.steps,conf.impedance,conf.volume,conf.balancechan,conf.balanceval,conf.temperature);	
			break;
		case MUTE_VOLUME:
			snprintf(cmd,MAXLEN,"%s","MUTE_VOLUME");	
			break;
		case CALIB_STATUS:
			snprintf(cmd,MAXLEN,"%s","CALIB_STATUS");	
			break;
		case CALIB_STOP:
			snprintf(cmd,MAXLEN,"%s","CALIB_STOP");	
			break;
		case INC_VOLUME:
			snprintf(cmd,MAXLEN,"%s","INC_VOLUME");	
			break;
		case DEC_VOLUME:
			snprintf(cmd,MAXLEN,"%s","DEC_VOLUME");	
			break;

	}	
	//printf("the command is %s\n",cmd);
#if 1
	
	
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	    perror("socket");
	    exit(1);
	}
	
	//printf("Trying to connect... %s\n",SOCK_PATH);

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
	    perror("connect");
	    exit(1);
	
	}

	//printf("Connected.\n");
	
	printf("Request: %s \n",cmd);
	send(s,cmd,MAXLEN,0);	
	if(command == CALIB_STATUS) {
		memset(cmd,'\0',MAXLEN);
		recv(s,cmd,MAXLEN,0);
		printf("Response: %s \n",cmd);

	}
	

	close(s);
#endif
	return 0;
}
