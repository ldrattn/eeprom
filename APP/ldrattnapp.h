#ifndef LDRATTNAPP_H 
#define LDRATTNAPP_H

#define VOL_MAX_STEP 102     //** maximum volume steps; range: 20...80. Higher = more memory usage
#define MAXLEN 80
#define DBMAX 48
#define MUTE 0 
#define CALIB_MAXSTEP  100 

#define MIN_VOLUME_STEPS 48     //** minimum volume steps;

#define HEADER_OFFSET 0x00
#define HEADER_LEN 0x64
#define CSV_OFFSET 0x800

#define CSVHEADER_LEN 0x8
#define PSTFLG_OFFSET 0x0 
#define IMP_OFFSET 0x1 
#define STEPS_OFFSET 0x2 
#define ADDR_OFFSET 0x3 

#define CSV_FILEDS 10
#define CSVFILE_HEADER 3 
typedef enum { CALIBRATED, NOTCALIBRATED } Calibstate;
typedef enum { CALIB_NONE, CALIB_ONGOING,CALIB_SUCCESS,CALIB_FAILED} Calibstatus;
typedef enum { CRN_NONE, CRN_ONGOING} CRNstatus;


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

extern LDRAttr ldrattn;
//extern LDRdata dataL[VOL_MAX_STEP - 2]; // left channel
//extern LDRdata dataR[VOL_MAX_STEP - 2]; // right channel
//extern unsigned int Temp;
//extern unsigned int Impedance;


struct volumeattr {
	int volume;
	int balanceChan;
	int balanceValue;
};

struct calibattr {
	unsigned int impedance;
	int numsteps;
	int temperature;
	int status;
	int pipefd;
	int shstep;
	int sestep;
};

//extern unsigned int defVolume;
//extern unsigned int impedance;
//extern unsigned int calibSteps;
typedef struct {
	unsigned int volume;
	unsigned int impedance;
	unsigned int calibSteps;
	unsigned int balanceChan; 
	unsigned int balanceValue; 
	unsigned int deltaTemp; 
}LDRConf;
 
extern LDRConf ldrconf; 
extern unsigned int calibStop;
extern unsigned int deltaStop;

#ifndef TRUE
#  define       TRUE    (1==1)
#  define       FALSE   (1==2)
#endif

int readCalibData(char *); 
void setVolume(int );
//int doCalibration(unsigned int ,int ,int ); 
void loadDefparams(void);
int readParseCfg(void);

#endif 
