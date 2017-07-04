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

unsigned int len;
#define VOL_MAX_STEP 102
#define MAXLEN 1024
#define MAXFIELDS 9 

struct eeprom
{
        char *dev;      // device file i.e. /dev/i2c-N
        int addr;       // i2c address
        int fd;         // file descriptor
        int write_cycle_time;
};

typedef struct {
        unsigned short pw_SE;            // series
        unsigned short pw_SH;            // shunt
        unsigned short i_SE;            // current series
        unsigned short i_SH;            // current shunt
} LDRdata;

typedef struct {
        unsigned short series;
        unsigned short shunt;
} LDRtargetres;

typedef struct {
        LDRdata dataL[VOL_MAX_STEP - 2];
        LDRdata dataR[VOL_MAX_STEP - 2];
        LDRtargetres targetres[VOL_MAX_STEP - 2];
        unsigned short ldrTemp;      //
        unsigned short potImpedence;
        unsigned short calibSteps;
}LDRAttr;


LDRAttr ldrattn;


unsigned short DataRISH[] = { 9321,100,25,\
65535,0,0,0,65535,0,0,0,0,10000,\
20411,30000,24049,6567,0,0,0,0,23174,23958,\
0,65535,24049,6561,0,0,0,0,23595,20716,\
20411,30000,24049,6565,0,0,0,0,22630,23974,\
20411,29878,24049,7185,0,0,0,0,0,65535,\
20411,29756,24049,7804,0,0,0,0,9950,49,\
20411,29512,24049,8968,0,0,0,0,9947,52,\
20411,29306,24049,9933,0,0,0,0,9944,55,\
20411,29080,24049,10919,20411,29080,24049,10919,9941,58,\
20411,28817,24049,12042,20411,28817,24049,12042,9937,62,\
20411,28691,24049,12544,20411,28691,24049,12544,9934,65,\
20411,28450,24049,13483,20411,28450,24049,13483,9930,69,\
20411,28282,24049,14114,20411,28282,24049,14114,9926,73,\
20411,28117,24049,14692,20411,28117,24049,14692,9922,77,\
20412,27898,24049,15400,20412,27898,24049,15400,9917,82,\
20412,27756,24049,15861,20412,27756,24049,15861,9913,86,\
20412,27571,24048,16416,20412,27571,24048,16416,9908,91,\
20412,27387,24048,16933,20412,27387,24048,16933,9902,97,\
20412,27259,24049,17296,20412,27259,24049,17296,9897,102,\
20412,27095,24048,17714,20412,27095,24048,17714,9891,108,\
20412,26951,24048,18077,20412,26951,24048,18077,9885,114,\
20412,26784,24048,18449,20412,26784,24048,18449,9878,121,\
20413,26625,24048,18800,20413,26625,24048,18800,9871,128,\
20413,26499,24048,19081,20413,26499,24048,19081,9864,135,\
20414,26354,24048,19362,20414,26354,24048,19362,9856,143,\
20414,26230,24048,19607,20414,26230,24048,19607,9848,151,\
20415,26087,24048,19870,20415,26087,24048,19870,9839,160,\
20416,25963,24047,20086,20416,25963,24047,20086,9830,169,\
20416,25841,24047,20290,20416,25841,24047,20290,9820,179,\
20417,25712,24047,20492,20417,25712,24047,20492,9809,190,\
20418,25604,24047,20656,20418,25604,24047,20656,9799,200,\
20419,25478,24047,20837,20419,25478,24047,20837,9787,212,\
20420,25365,24046,20993,20420,25365,24046,20993,9775,224,\
20421,25245,24045,21146,20421,25245,24045,21146,9762,237,\
20423,25132,24044,21286,20423,25132,24044,21286,9748,251,\
20424,25025,24045,21412,20424,25025,24045,21412,9734,265,\
20425,24914,24043,21538,20425,24914,24043,21538,9719,280,\
20426,24811,24044,21651,20426,24811,24044,21651,9703,296,\
20428,24703,24043,21763,20428,24703,24043,21763,9685,314,\
20429,24609,24042,21863,20429,24609,24042,21863,9667,332,\
20669,21823,23961,23575,20669,21823,23961,23575,7230,2804,\
20689,21771,23954,23595,20689,21771,23954,23595,7071,2967,\
20710,21714,23947,23616,20710,21714,23947,23616,6903,3140,\
20733,21661,23940,23637,20733,21661,23940,23637,6725,3323,\
20758,21613,23930,23655,20758,21613,23930,23655,6537,3517,\
20784,21556,23922,23676,20784,21556,23922,23676,6338,3722,\
20813,21496,23912,23699,20813,21496,23912,23699,6128,3940,\
20845,21440,23901,23720,20845,21440,23901,23720,5906,4170,\
20884,21389,23886,23740,20884,21389,23886,23740,5671,4414,\
20925,21335,23871,23761,20925,21335,23871,23761,5422,4674,\
20969,21283,23857,23780,20969,21283,23857,23780,5159,4948,\
21018,21234,23839,23797,21018,21234,23839,23797,4882,5238,\
21075,21183,23820,23815,21075,21183,23820,23815,4588,5547,\
21140,21133,23796,23833,21140,21133,23796,23833,4277,5874,\
21215,21084,23769,23852,21215,21084,23769,23852,3949,6221,\
21303,21035,23738,23870,21303,21035,23738,23870,3601,6589,\
21406,20990,23700,23885,21406,20990,23700,23885,3234,6979,\
21535,20947,23651,23899,21535,20947,23651,23899,2846,7393,\
21695,20898,23590,23913,21695,20898,23590,23913,2435,7833,\
21904,20850,23507,23930,21904,20850,23507,23930,2001,8299,\
22197,20809,23385,23944,22197,20809,23385,23944,1541,8796,\
0,65535,9950,49,0,65535,24049,6561,25,100
};


#define die_if3(a, msg, code) do { do_die_if( a , msg, __LINE__, code); } while(0)
#define die_if(a, msg) die_if3(a, msg, 1)
static void do_die_if(int b, char* msg, int line, int exitcode)
{
        if(!b)
                return;
        fprintf(stderr, "Error at line %d: %s\n", line, msg);
        //fprintf(stderr, "     sysmsg: %s\n", strerror(errno));
        exit(exitcode);
}


int eeprom_write_byte(struct eeprom *e, unsigned short mem_addr, char data)
{
	
	char reg;
	unsigned short value;
	int ret;
	
	reg = (mem_addr >> 8) & 0x00ff;
	value = mem_addr & 0x00ff;	
	value = (data << 8) | value;

	ret = wiringPiI2CWriteReg16(e->fd,reg,value);
	if(ret < 0) {
		printf("error while write data to eeprom\n");
	}
	usleep(3000);
	return ret;	

}

int eeprom_read_byte(struct eeprom *e, unsigned short mem_addr)
{
	char reg;
	char value;
	int ret;

	ioctl(e->fd, BLKFLSBUF);
		
	reg = (mem_addr >> 8) & 0x0ff;
	value = mem_addr & 0x0ff;
	//usleep(1000);	
	ret = wiringPiI2CWriteReg8(e->fd,reg,value);
	//ret = wiringPiI2CWriteReg8(e->fd,0x0,0x0);
	if(ret < 0) {
		printf("error while write addr to eeprom\n");
		return ret; 
	}
	usleep(10);	
	ret = wiringPiI2CRead(e->fd);
	//printf("the read data is 0x%x\n",ret);
	return ret;
 
}

#if 1
int write_to_eeprom_from_array(struct eeprom *e,int addr)
{
        int i;
        unsigned short data;

        len = sizeof(DataRISH)/sizeof(DataRISH[0]);
        printf("array len is %d\n",len);
        for(i=0;i<len;i++) {
                data = DataRISH[i] ;
                //printf("data  0x%x\n",data);
		die_if(eeprom_write_byte(e, addr++, (data >> 8)), "write error");
		//usleep(5000);	
		die_if(eeprom_write_byte(e, addr++, data), "write error");
        }
        printf("\n\n");
        return 0;
}

static int read_from_eeprom(struct eeprom *e, FILE *fp, int addr, int size, int hex)
{
        int ch, i;
        unsigned short value= 0x0;
        int row = 0;
        int col = 0;
        char buf[MAXLEN];

        printf("Redaing from eeprom %d %d \n",addr,size);

        while(row < (VOL_MAX_STEP-2))
        {
                value = 0x0;
                die_if((ch = eeprom_read_byte(e, addr++)) < 0, "read error");
                value = (ch << 8) | value;
                die_if((ch = eeprom_read_byte(e, addr++)) < 0, "read error");
                value = value | ch;
                //printf("the read value is 0x%x  row %d col %d  \n",value,row,col);
                if(row == 0) {
                        switch(col) {
                                case 0:
                                                ldrattn.potImpedence = value;
                                                col++;
                                                break;
                                case 1:
                                                ldrattn.calibSteps = value;
                                                col++;
                                                break;
                                case 2:
                                                ldrattn.ldrTemp = value;
                                                col = 0;
                                                row++;
                                                break;

                        }

                } else {
                        switch(col) {

                                case 0: ldrattn.dataR[row].pw_SE = value;
                                break;
                                case 1: ldrattn.dataR[row].pw_SH = value;
                                break;
                                case 2: ldrattn.dataR[row].i_SE = value;
                                break;
                                case 3: ldrattn.dataR[row].i_SH = value;
                                break;
                                case 4: ldrattn.dataL[row].pw_SE = value;
                                break;
                                case 5: ldrattn.dataL[row].pw_SH = value;
                                break;
                                case 6: ldrattn.dataL[row].i_SE = value;
                                break;
                                case 7: ldrattn.dataL[row].i_SH = value;
                                break;
                                case 8: ldrattn.targetres[row].series = value;
                        break;
                                case 9: ldrattn.targetres[row].shunt = value;
                        break;

                        }
                        if(col >= MAXFIELDS) {
                                col = 0;
                                //i = row;
                        //printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",ldrattn.dataR[i].pw_SE,ldrattn.dataR[i].pw_SH,ldrattn.dataR[i].i_SE,ldrattn.dataR[i].i_SH,ldrattn.dataL[i].pw_SE,ldrattn.dataL[i].pw_SH,ldrattn.dataL[i].i_SE,ldrattn.dataL[i].i_SH,ldrattn.targetres[i].series,ldrattn.targetres[i].shunt);
                        row++;
                        } else
                                col++;
                }

        }

        printf(" writing to file num of rows %d \n",row);

        sleep(1);
        memset(buf,'\0',MAXLEN);
        sprintf(buf,"%d,%d,%d,\n",ldrattn.potImpedence,ldrattn.calibSteps,ldrattn.ldrTemp);
        printf("%d,%d,%d,\n",ldrattn.potImpedence,ldrattn.calibSteps,ldrattn.ldrTemp);
        fwrite(buf,sizeof(buf),1,fp);
        //printf(" %s\n",buf);
        for(i =1 ;i < (row-1);i++) {
                memset(buf,'\0',MAXLEN);
                sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",ldrattn.dataR[i].pw_SE,ldrattn.dataR[i].pw_SH,ldrattn.dataR[i].i_SE,ldrattn.dataR[i].i_SH,ldrattn.dataL[i].pw_SE,ldrattn.dataL[i].pw_SH,ldrattn.dataL[i].i_SE,ldrattn.dataL[i].i_SH,ldrattn.targetres[i].series,ldrattn.targetres[i].shunt);
                fwrite(buf,sizeof(buf),1,fp);
                //printf(" %s\n",buf);
//              printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",ldrattn.dataR[i].pw_SE,ldrattn.dataR[i].pw_SH,ldrattn.dataR[i].i_SE,ldrattn.dataR[i].i_SH,ldrattn.dataL[i].pw_SE,ldrattn.dataL[i].pw_SH,ldrattn.dataL[i].i_SE,ldrattn.dataL[i].i_SH,ldrattn.targetres[i].series,ldrattn.targetres[i].shunt);
        }

        fflush(fp);
        return 0;
}
#endif



int eeprom_close(struct eeprom *e)
{

	e->fd = -1;
	e->dev = 0;
	return 1;
}

int eeprom_open (char *dev,int i2cAddr,struct eeprom *e,int write_cycle_time)
{
	int fd;

	if ((fd = wiringPiI2CSetup (i2cAddr)) < 0) {
		printf("error opening the eeprom device\n");
		return 0;
	}
	
	e->fd = fd;
	e->addr = i2cAddr;
	e->dev = dev;
	e->write_cycle_time = write_cycle_time;	

	return 1;
}

#if 1
int main()
{
    int data;
	struct eeprom e;
	int ret;
	FILE *output_fp;
	output_fp = fopen("testing.csv", "w");
			

	wiringPiSetup () ;
	ret =  eeprom_open("/dev/i2c-2",0x57,&e,5);		
	if(!ret)
		return 1;
	
	//eeprom_write_byte(&e,0x0,0xda);
	//eeprom_read_byte(&e,0x0);

	write_to_eeprom_from_array(&e,0x0);

	read_from_eeprom(&e,output_fp,0x00,2048,1);
	
	fclose(output_fp);
	eeprom_close(&e);		

	
	return 1;

}
#endif


