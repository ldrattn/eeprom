
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "logger.h"

const char *sysLogFile = "/var/log/ldrattnapp.log";



void openLog()
{
	openlog(sysLogFile, LOG_PID|LOG_CONS|LOG_NDELAY, LOG_USER);	
}

void debugLog(int level,char *fmt,...)
{
	syslog(level,fmt);
}

void closeLog()
{
	closelog();
}
