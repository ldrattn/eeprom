/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include <sys/stat.h>
	
#include "csv.h"
#include "ldrattnapp.h"
#include "debug.h"

#define MAX_FIELDS 9 
#define MAX_BUF_LEN 1024

unsigned int count;
unsigned int db;
unsigned int flag;

LDRAttr ldrattn;
LDRConf ldrconf;


void getCSVFilename(unsigned int impedance,char *csvfile)
{
	char filename[MAXLEN];
	const char *csvpath = "/usr/local/etc/ldrattn/csv/";
	
	memset(filename,'\0',MAXLEN);		
	sprintf(filename,"%s%dk.csv",csvpath,impedance);			

	strcpy(csvfile,filename);	
	LOG_TRACE(LOG_INFO,"the csvfile %s\n",csvfile);		 			
}

void storeLDRBuffer(int value) {

	if(!flag){
		switch(count) {
			case 0: ldrattn.potImpedence = value;
				count = 1; 
				break;	
			case 1: ldrattn.calibSteps = value;
				count = 2; 
				break;	
			case 2: ldrattn.ldrTemp = value;
				flag = 1;
				count = 0;
				break;		
		}
	} else {
		switch(count) {
		case 0: ldrattn.dataR[db].pw_SE = value;
			break;	
		case 1: ldrattn.dataR[db].pw_SH = value;
			break;	
		case 2: ldrattn.dataR[db].i_SE = value;
			break;	
		case 3: ldrattn.dataR[db].i_SH = value;
			break;		
		case 4: ldrattn.dataL[db].pw_SE = value;
			break;		
		case 5: ldrattn.dataL[db].pw_SH = value;
			break;		
		case 6: ldrattn.dataL[db].i_SE = value;
			break;		
		case 7: ldrattn.dataL[db].i_SH = value;
			break;		
		case 8: ldrattn.targetres[db].series = value;
				break;		
		case 9: ldrattn.targetres[db].shunt = value;
				break;		

		}
		if(count >= MAX_FIELDS) {
			count = 0;
			db++;
		} else
			count++;	

	//	if(db > ldrattn.calibSteps)	
	//		db = 0;	

	}	

}


void callback1 (void *s, size_t i, void *outfile) {
	printf("data read %d  count  %d  %d db  %d \n",atoi((char *)s),count,i,db );
	//Todo : optimise the logic


//Todo, csvwrite should clear the struct members before writing the actual data.
//remove the else after doing this
	int value =  atoi((char *)s);
	if(value ==  808464432)	
		value = 0;

#if 0
	if(!flag){
		switch(count) {
			case 0: ldrattn.potImpedence = atoi((char *)s);
				count = 1; 
				break;	
			case 1: ldrattn.calibSteps = atoi((char *)s);
				count = 2; 
				break;	
			case 2: ldrattn.ldrTemp = atoi((char *)s);
				flag = 1;
				count = 0;
				break;		
		}
	} else {
		switch(count) {
		case 0: ldrattn.dataR[db].pw_SE = atoi((char *)s);
			break;	
		case 1: ldrattn.dataR[db].pw_SH = atoi((char *)s);
			break;	
		case 2: ldrattn.dataR[db].i_SE = atoi((char *)s);
			break;	
		case 3: ldrattn.dataR[db].i_SH = atoi((char *)s);
			break;		
		case 4: ldrattn.dataL[db].pw_SE = atoi((char *)s);
			break;		
		case 5: ldrattn.dataL[db].pw_SH = atoi((char *)s);
			break;		
		case 6: ldrattn.dataL[db].i_SE = atoi((char *)s);
			break;		
		case 7: ldrattn.dataL[db].i_SH = atoi((char *)s);
			break;		
		case 8: ldrattn.targetres[db].series = atoi((char *)s);
				break;		
		case 9: ldrattn.targetres[db].shunt= atoi((char *)s);
				break;		

		}
		if(count >= MAX_FIELDS) {
			count = 0;
			db++;
		} else
			count++;	

	//	if(db > ldrattn.calibSteps)	
	//		db = 0;	

	}
#else
	storeLDRBuffer(value);	
#if 0	
	if(!flag){
		switch(count) {
			case 0: ldrattn.potImpedence = value;
				count = 1; 
				break;	
			case 1: ldrattn.calibSteps = value;
				count = 2; 
				break;	
			case 2: ldrattn.ldrTemp = value;
				flag = 1;
				count = 0;
				break;		
		}
	} else {
		switch(count) {
		case 0: ldrattn.dataR[db].pw_SE = value;
			break;	
		case 1: ldrattn.dataR[db].pw_SH = value;
			break;	
		case 2: ldrattn.dataR[db].i_SE = value;
			break;	
		case 3: ldrattn.dataR[db].i_SH = value;
			break;		
		case 4: ldrattn.dataL[db].pw_SE = value;
			break;		
		case 5: ldrattn.dataL[db].pw_SH = value;
			break;		
		case 6: ldrattn.dataL[db].i_SE = value;
			break;		
		case 7: ldrattn.dataL[db].i_SH = value;
			break;		
		case 8: ldrattn.targetres[db].series = value;
				break;		
		case 9: ldrattn.targetres[db].shunt = value;
				break;		

		}
		if(count >= MAX_FIELDS) {
			count = 0;
			db++;
		} else
			count++;	

	//	if(db > ldrattn.calibSteps)	
	//		db = 0;	

	}	
#endif
#endif
}

int readCalibData(char *Fname) 
{
	struct csv_parser p;
	FILE *infile;
	char buf[MAX_BUF_LEN];
	unsigned int Nbytes;
	struct stat st;

	if(!strlen(Fname)) {
		printf("Filename is Null\n");
		return EXIT_FAILURE;
	}

	stat(Fname,&st);

	if(!st.st_size){
		printf("%s csv file is empty\n",Fname);
		return EXIT_FAILURE;
	}		

	csv_init(&p, CSV_APPEND_NULL | CSV_STRICT);

	infile = fopen(Fname, "rb");
	if (infile == NULL) {
		printf("Failed to open the file %s\n",Fname);
		return EXIT_FAILURE;
	}
	flag = 0;
	count = 0; db=0;
	while ((Nbytes = fread(buf, 1, MAX_BUF_LEN, infile)) > 0) {
		if (csv_parse(&p, buf, Nbytes, callback1, NULL,NULL) != Nbytes) {
			printf("Error parsing file: %s\n", csv_strerror(csv_error(&p)));
			fclose(infile);
			return EXIT_FAILURE;
		}
	}

	csv_fini(&p, callback1, NULL, NULL);
	csv_free(&p);

	fclose(infile);

	return EXIT_SUCCESS;
} 


//int writeCalibData(char *Fname,unsigned int steps,unsigned int temperature,LDRAttr ldrattn_t)
int writeCalibData(char *Fname,LDRAttr ldrattn_t,int dataflag)
{
	struct csv_parser p;
	FILE *outfile;
	char buf[MAX_BUF_LEN];
	unsigned int i;

	if(!strlen(Fname)) {
		printf("Filename is Null\n");
		return EXIT_FAILURE;
	}

	if(!dataflag) {
		ldrattn_t = ldrattn;			
	} 
	
	csv_init(&p, 0);

	outfile = fopen(Fname, "wb");
	if (outfile == NULL) {
		printf("Failed to open the file %s\n",Fname);
		return EXIT_FAILURE;
	}
	
	sprintf(buf,"%d,%d,%d,\n",ldrattn_t.potImpedence,ldrattn_t.calibSteps,ldrattn_t.ldrTemp);
	csv_fwrite(outfile,buf,strlen(buf));	
	for(i =0 ;i <= ldrattn_t.calibSteps;i++) {
		sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",ldrattn_t.dataR[i].pw_SE,ldrattn_t.dataR[i].pw_SH,ldrattn_t.dataR[i].i_SE,ldrattn_t.dataR[i].i_SH,ldrattn_t.dataL[i].pw_SE,ldrattn_t.dataL[i].pw_SH,ldrattn_t.dataL[i].i_SE,ldrattn_t.dataL[i].i_SH,ldrattn_t.targetres[i].series,ldrattn_t.targetres[i].shunt);	
		csv_fwrite(outfile,buf,strlen(buf));	
	}

	csv_fini(&p, callback1, NULL, NULL);
	csv_free(&p);

	fclose(outfile);
	return EXIT_SUCCESS;
}


