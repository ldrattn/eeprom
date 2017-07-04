/*
 * parse: parse simple name/value pairs
 *
 * SAMPLE BUILD:
 * cc -g -Wall -o parse parse.c
 *
 * SAMPLE OUTPUT:
 * ./parse =>
 *   Initializing parameters to default values...
 *   Reading config file...
 *   Final values:
 *     item: cone, flavor: vanilla, size: large
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/stat.h>	
#include "ldrattnapp.h"
#include "debug.h"
#include "logger.h"
#include "common.h"
#define MAXBUF 256

char *LDR_CONF = "/usr/local/etc/ldrattn/ldrattn.conf";
LDRConf ldrconf;

/*
 * initialize data to default values
 */
void loadDefParams()
{
	ldrconf.volume = DEFAULT_VOLUME;	
	ldrconf.impedance = DEFAULT_IMPEDANCE;	
	ldrconf.calibSteps = DEFAULT_CALIB_STEPS;	
	ldrconf.balanceChan = LEFTCHAN; //left
	ldrconf.balanceValue = DEFAULT_BALANCE_VALUE_DB;
	
}

void removespace (char * s)
{
	/* Initialize start, end pointers */
	char *s1 = s, *s2 = &s[strlen (s) - 1];

	/* Trim and delimit right side */
	while ( (isspace (*s2)) && (s2 >= s1) )
  		s2--;
	*(s2+1) = '\0';

	/* Trim left side */
	while ( (isspace (*s1)) && (s1 < s2) )
		s1++;

	/* Copy finished string */
	strcpy(s,s1);
	LOG_TRACE(LOG_INFO,"converted string %s \n",s);
}

/*
 * parse external parameters file
 *
 * NOTES:
 * - There are millions of ways to do this, depending on your
 *   specific needs.
 *
 * - In general:
 *   a) The client will know which parameters it's expecting
 *      (hence the "struct", with a specific set of parameters).
 *   b) The client should NOT know any specifics about the
 *      configuration file itself (for example, the client
 *      shouldn't know or care about it's name, its location,
 *      its format ... or whether or not the "configuration
 *      file" is even a file ... or a database ... or something
 *      else entirely).
 *   c) The client should initialize the parameters to reasonable
 *      defaults
 *   d) The client is responsible for validating whether the
 *      pararmeters are complete, or correct.
 */
int readParseCfg()
{
	char *s, buff[MAXBUF];
	struct stat st;

	FILE *fp = fopen (LDR_CONF, "r");
	if (fp == NULL)
	{
		return EXIT_FAILURE;
	}

	stat(LDR_CONF,&st);

	if(!st.st_size){
		debugLog(LOG_INFO, "%s config is empty\n",LDR_CONF);
		
		return EXIT_FAILURE;
	}		

	/* Read next line */
	while ((s = fgets (buff, sizeof buff, fp)) != NULL)
	{
		/* Skip blank lines and comments */
		if (buff[0] == '\n' || buff[0] == '#')
			continue;
		/* Parse name/value pair from line */
		char name[MAXLEN], value[MAXLEN];
		int rvalue;
		s = strtok (buff, "=");
		if (s==NULL)
			continue;
		else {
			strncpy (name, s, MAXLEN);
			removespace(name);
		}

		s = strtok (NULL, "=");

		if (s==NULL)
			continue;
		else{
			strncpy (value, s, MAXLEN);
		}

		removespace(value);
		rvalue = atoi(value);
	
		/* Copy into correct entry in parameters struct */
		if (strcmp(name, "defVolume")==0)
			ldrconf.volume = rvalue;
		else if (strcmp(name, "impedance")==0)
			ldrconf.impedance = rvalue;
		else if (strcmp(name, "calibSteps")==0)
			ldrconf.calibSteps = rvalue;
		else if (strcmp(name, "balance")==0)
			ldrconf.balanceChan = rvalue;
		else if (strcmp(name, "balValue")==0)
			ldrconf.balanceValue = rvalue;
		else
			LOG_TRACE(LOG_INFO,"WARNING: %s/%d: Unknown name/value pair!\n", name, rvalue);
		LOG_TRACE(LOG_INFO,"the read values %s : %d \n",name,rvalue);
	}

	/* Close file */
	fclose (fp);

	return EXIT_SUCCESS;
}

int writeLDRConfig(char **argc)
{
	char buf[MAXBUF];
	unsigned int steps;
	unsigned int impedance;
	unsigned int volume;
	unsigned int balancechan;
	unsigned int balancevalue;
	unsigned int temperature;
	printf("here we r %d %s\n",__LINE__,__FILE__);

	steps = atoi(argc[1]); 	
	printf("here we r %d %s\n",__LINE__,__FILE__);
	impedance = atoi(argc[2]);
	printf("here we r %d %s\n",__LINE__,__FILE__);
	volume = atoi(argc[3]);
	balancechan = atoi(argc[4]);
	balancevalue = atoi(argc[5]);
	temperature = atoi(argc[6]); 


	FILE *fp = fopen (LDR_CONF, "w");
	if (fp == NULL)
	{
		return EXIT_FAILURE;
	}

	
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"defVolume=%d\n",volume);
	fputs(buf,fp);
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"impedance=%d\n",impedance);
	fputs(buf,fp);
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"calibSteps=%d\n",steps);
	fputs(buf,fp);
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"balance=%d\n",balancechan);
	fputs(buf,fp);
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"balValue=%d\n",balancevalue);
	fputs(buf,fp);
	memset(buf,'\0',MAXBUF);
	snprintf(buf,MAXBUF,"temperature=%d\n",temperature);
	fputs(buf,fp);

	fclose(fp);
	return EXIT_SUCCESS;
}
