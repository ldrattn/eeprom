#include <stdio.h>
#include <stdint.h>
#include <byteswap.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "ltc2655.h"

int fd ;

int writeDigitalData (int channel, int val)
{
	char config;
	int res;
	
	if (val < 0 || val >= 65536)
		return -1;

	//printf("writing val:%ld on dac chan:%d\n", val, channel);

	config = (channel & 0xf) | ((LTC2655_CMD_WR_UP_N & 0x0f) << 4);


	//val = ((val >> 8) & 0xff) | (val & 0xff);

	//printf("writing val:%04X on dac chan:%02X   config %02X \n", val, channel,config);
	val = __bswap_16 (val) ;	
	res = wiringPiI2CWriteReg16 (fd, config,val) ;
	//printf("writing val:%04X on dac chan:%02X   config %02X res %d  fd %d\n", val, channel,config,res,fd);
	if (res < 0) {
		return res;
	} else if (res != 3) {
		return -1;
	} else {
		return 0;
	}

}

void writeCommand (int channel, int command)
{
	char config;
	/*char command;	

	if(mode == 1)	
		command = LTC2655_CMD_EXTERNAL_REF;	
	else
		command = LTC2655_CMD_INTERNAL_REF;	
	*/
	config = (channel & 0xf) | ((command & 0x0f) << 4);

	//printf("writing channel:%02X config:%02X command %02X \n",  channel,config,command);

	wiringPiI2CWriteReg16 (fd, config,0x0) ;

}

/*
 * ltc2655Setup:
 *      Create a new wiringPi setup for DAC ltc2655 on the Pi's
 *      I2C interface.
 *********************************************************************************
 */

int ltc2655Setup (int i2cAddr)
{

	if ((fd = wiringPiI2CSetup (i2cAddr)) < 0) {
		return FALSE;
	}

	return TRUE;
}

#if 0
int main()
{
        int data;
	wiringPiSetup () ;
	ltc2655Setup(0x10);
	writeCommand(0xf,LTC2655_CMD_EXTERNAL_REF);
	writeDigitalData(2,65535);
	writeDigitalData(3,65535);
	return 1;

}
#endif


