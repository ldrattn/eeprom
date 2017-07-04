#include <stdio.h>
#include <stdint.h>
#include <byteswap.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "ldrattnapp.h"

unsigned int len;
//#define VOL_MAX_STEP 102
//#define MAXLEN 1024
//#define MAXFIELDS 9 

//LDRAttr ldrattn;
#if 0
struct eeprom
{
        char *dev;      // device file i.e. /dev/i2c-N
        int addr;       // i2c address
        int fd;         // file descriptor
        int write_cycle_time;
};
#endif


int readHeaders(char *csvheader,int eepromfd)
{
	unsigned int i ;	
	unsigned int ret ;	
	unsigned int addr;
	
	for(i=0;i<HEADER_LEN;i++) {
		int addr = (HEADER_OFFSET+i);		
		//if( (ret = eeprom_read_byte(eepromfd,(HEADER_OFFSET+i))) < 0 ) {
		ret = eeprom_read_byte(eepromfd,addr);	
		if(ret < 0 ) {
			return ret;
		} else {
			*(csvheader+i) = ret;
		}
	}
		
}

int getCSVFlag(int offset,char *csvheader)
{
	return *(csvheader+offset);	
}


#define getImpedance(offset) *(csvheader+offset+IMP_OFFSET) 
#define getAddr(offset) *(csvheader+offset+IMP_OFFSET) 
#define getSteps(offset) *(csvheader+offset+IMP_OFFSET) 
#define getcsvlen(steps) (((steps*CSV_FIELDS)+CSVFILE_HEADER)*2)

int rwEEPROMCSV(int offset,char *csvheader,int eepromfd)
{
	int ret;
	unsigned int impedance;
	unsigned int i;
	unsigned int addr;
	unsigned int steps;
	int value;
	char csvfile[MAXLEN];

	impedance = getImpedance(offset);
	addr = getAddr(offset);
	steps = getSteps(offset);
	steps++;

	for(i=0;i<getCSVLen(steps);i++) {
		value = eeprom_read_byte(eepromfd,addr++);
		if( value < 0 ) {
			printf("error while reading eeprom data \n");
			return value;
		}					
		storeLDRBuffer(value);	
	}
	
	getCSVFilename(impedance,csvfile);
	remove(csvfile);	
	if((ret = writeCalibData(csvfile,NULL,0)) == EXIT_FAILURE) {
		printf("Failed to wrtite the eeprom data to csv file\n");
	}
	return ret; 
		
}

int eeprom_write_byte(int eepromfd, unsigned short mem_addr, char data)
{
	
	char reg;
	unsigned short value;
	int ret;
	
	reg = (mem_addr >> 8) & 0x00ff;
	value = mem_addr & 0x00ff;	
	value = (data << 8) | value;

	ret = wiringPiI2CWriteReg16(eepromfd,reg,value);
	if(ret < 0) {
		printf("error while write data to eeprom\n");
	}
	usleep(3000);
	return ret;	

}

int eeprom_read_byte(int eepromfd, unsigned short mem_addr)
{
	char reg;
	char value;
	int ret;

	ioctl(eepromfd, BLKFLSBUF);
		
	reg = (mem_addr >> 8) & 0x0ff;
	value = mem_addr & 0x0ff;
	//usleep(1000);	
	ret = wiringPiI2CWriteReg8(eepromfd,reg,value);
	if(ret < 0) {
		printf("error while write addr to eeprom\n");
		return ret; 
	}
	usleep(10);	
	ret = wiringPiI2CRead(eepromfd);
	//printf("the read data is 0x%x\n",ret);
	return ret;
 
}

#if 0
int eeprom_close(int eepromfd)
{

	eepromfd = -1;
	//e->dev = 0;
	return 1;
}
#endif

int eeprom_open (char *dev,int i2cAddr)
{
	int fd;

	if ((fd = wiringPiI2CSetup (i2cAddr)) < 0) {
		printf("error opening the eeprom device\n");
	}

	return fd;
}

int initEEPROM(int eepromfd) {
	int ret;

	ret = eeprom_open("/dev/i2c-2",0x57);		
	return ret;
}


