#include <linux/errno.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include<sys/types.h> 
#include<sys/stat.h> 
#include<fcntl.h>
#include<syslog.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <ads1115.h>
#include "action.h"
#include "debug.h"
#include "logger.h"
#include "ldrattnapp.h"
#include "csvparser.h"

#include "ltc2655.h"

LDRAttr ldrattn;
LDRAttr ldrattn_t;

unsigned int calibStop;
unsigned int deltaStop;
int LDRfd;

int set_dac_channel_raw(int channel,int val){

	int mappedchan;

	switch(channel){
	case PIN_DAC_LSE: 
		mappedchan = DAC_CHAN1;
		break;	
	case PIN_DAC_LSH:
		mappedchan = DAC_CHAN2;
		break;	
	case PIN_DAC_RSE:
		mappedchan = DAC_CHAN3;
		break;	
	case PIN_DAC_RSH:
		mappedchan = DAC_CHAN4;
		break;	
	default:
		printf("invalid ADC channel\n");
	}

	writeDigitalData(mappedchan,val);
	return 0;
} 

int setLSE(int val) 
{
	int ret;
	ret  = set_dac_channel_raw(PIN_DAC_LSE, val); 
	return ret;
}

int setRSE(int val) 
{
	int ret;
	ret  = set_dac_channel_raw(PIN_DAC_RSE, val); 
	return ret;
}

int setLSH(int val) 
{
	int ret;
	ret  = set_dac_channel_raw(PIN_DAC_LSH, val); 
	return ret;
}

int setRSH(int val) 
{
	int ret;
	ret  = set_dac_channel_raw(PIN_DAC_RSH, val); 
	return ret;
}


/******* get attenuation factor from step number, LOG law *******/
float getAttFromStep(float step) {
	float p ; //= step / (float)(VOL_MAX_STEP);
	p = step; 
	float tmpatt = pow(10,(-(p/20)));
	return tmpatt;

}

/******* read LDR resistor value *******/
unsigned long getResistance(int  measure_raw, int seriesLDRres, int refResistor) {
	float measure = (float) (measure_raw * 4096 ) / 32767 ;
	if (measure > 4096)
		return 4000000;
	else {
		float v = (float) measure / 1000;
		float result = ((v * refResistor) / (LDR_VOLTAGE - v)) - seriesLDRres;
	//	printf("measure_raw %d  measuremv:%f res:%f seriesLDR:%d \n",measure_raw, measure, ((v * refResistor) / (LDR_VOLTAGE - v)),seriesLDRres);	
		return (int long)(result);
	}
}

/******* get oversampled 12-bit reading *******/
unsigned int  getSample(int channel) {
	int  result = 0, res;
	int  i=0;
	int mappedchan;

	switch(channel){
	case PIN_ADC_LSE: 
		mappedchan = ADC_CHAN1;
		break;	
	case PIN_ADC_LSH:
		mappedchan = ADC_CHAN2;
		break;
	case PIN_ADC_RSE:
		mappedchan = ADC_CHAN3;
		break;	
	case PIN_ADC_RSH:
		mappedchan = ADC_CHAN4;
		break;	
	case PIN_CALIB_L:
		mappedchan = ADC_CHAN5;
		break;	
	case PIN_CALIB_R:
		mappedchan = ADC_CHAN6;
		break;	
	default:
		printf("invalid ADC channel\n");
	}
#if 0

	result = analogRead(mappedchan);
#else
	for(i =0; i<4; i++){
		res = analogRead(mappedchan);
		result += res;  ;
		usleep(100);
	}
	result = result >> 2 ; 
#endif 
	return result ;
}

unsigned int getILSE() {
	return getSample(PIN_ADC_LSE);
}

unsigned int  getILSH() {
	return getSample(PIN_ADC_LSH);
}

unsigned int getIRSE() {
	return getSample(PIN_ADC_RSE);
}

unsigned int getIRSH() {
	return getSample(PIN_ADC_RSH);
}

unsigned long getRLSE() {
	sleep(DELAY_LDR_STABLE);
	return getResistance(getSample(PIN_CALIB_L), LDR_LSH_MIN, LDR_R1);
}

unsigned long getRLSH() {
	sleep(DELAY_LDR_STABLE);
	return getResistance(getSample(PIN_CALIB_L), LDR_LSE_MIN, LDR_R1);
}

unsigned long getRRSE() {
	sleep(DELAY_LDR_STABLE);
	return getResistance(getSample(PIN_CALIB_R), LDR_RSH_MIN, LDR_R18);
}

unsigned long getRRSH() {
	sleep(DELAY_LDR_STABLE);
	return getResistance(getSample(PIN_CALIB_R), LDR_RSE_MIN, LDR_R18);
}

/******* get series R from attenuation and impedance *******/
unsigned long getRxFromAttAndImp(float att, unsigned long imp) {
	return (unsigned int long)(imp - att * imp);
}

/******* get shunt R from attenuation and impedance *******/
unsigned long getRyFromAttAndImp(float att, unsigned long imp) {
	return (unsigned int long)(att * imp);
}

/******* get series R from attenuation and shunt R *******/
unsigned long getRxFromAttAndRy(float att, unsigned long ry) {
	return (unsigned int long)(ry * (1 - att) / att);
}

/******* get shunt R from attenuation and series R *******/
unsigned long getRyFromAttAndRx(float att, unsigned long rx) {
	return (unsigned int long)(att * rx / (1 - att));
}

void setCalibrationRelays(unsigned int val) {

}


/** average difference in I (current) samples when PWM is increased by 1 **/
char getDeltaI(int sample) {
	int s = sample & 32767;
	if (sample < 32768) {
		if (s < 42) return 5;
		if (s < 57) return 6;
		if (s < 100) return 7;
		if (s < 150) return 8;
		if (s < 210) return 9;
		if (s < 500) return 10;
		else return 11;
	}
	else {
		if (s < 16) return 3;
		if (s < 32) return 4;
		if (s < 50) return 5;
		if (s < 70) return 6;
		if (s < 100) return 7;
		if (s < 180) return 8;
		if (s < 300) return 9;
		if (s < 640) return 10;
		else return 11;
	}
}

void deltaCorrection(struct volumeattr *attr)
{

	int deltaSample;
	int deltaPW;
	int sample;
	int lVolume = attr->volume;
	int rVolume = attr->volume;

	(attr->balanceChan == 0)?(lVolume+=attr->balanceValue):(rVolume+=attr->balanceValue);
	//while(1)
	LOG_TRACE(LOG_INFO," lvolume %d rvolume %d\n",lVolume,rVolume);
	while(1) {
		if(deltaStop) {
			deltaStop = FALSE;
			pthread_exit(0);
		}
	

	usleep(200*1000);
	//sleep(10);

#if 0
		// LSE
		sample = getILSE();
		deltaSample = (int)(dataL[volume - 2].i_SE & 32767) - sample;
		deltaPW = deltaSample > 0 ? 1 : -1;
		if (deltaPW * deltaSample > abs(deltaPW * getDeltaI(dataL[volume - 2].i_SE) - deltaSample) && dataL[volume - 2].pw_SE > 0 && dataL[volume - 2].pw_SE < 65535) {
			dataL[volume - 2].pw_SE += deltaPW;
			setLSE(dataL[volume - 2].pw_SE);
		}

#endif
#if 0
		// LSE
		sample = getILSE();
//		printf("ilse:%d iseold:%d\n",sample, dataL[volume - 2].i_SE);
		deltaSample = (int)(dataL[volume - 2].i_SE & 32767) - sample;
		deltaPW = deltaSample > 0 ? -1 : 1;
//		printf("leftse deltaPW:%d deltaSample:%d\n",deltaPW, deltaSample);
//		printf("1:%d : %d\n", (deltaPW * deltaSample), abs(deltaPW * getDeltaI(dataR[volume - 2].i_SE) - deltaSample));
//		if (deltaPW * deltaSample > abs(deltaPW * getDeltaI(dataR[volume - 2].i_SE) - deltaSample) && dataR[volume - 2].pw_SE > 0 && dataR[volume - 2].pw_SE < 65535) {
		if(abs(deltaSample)>2){
			dataL[volume - 2].pw_SE += deltaPW;
			setLSE(dataL[volume - 2].pw_SE);
		}
		printf("%04d lse DAC:%ld ADC:%d ",i++ ,dataL[volume - 2].pw_SE,sample);
#endif
#if 1	
		// RSE
		sample = getIRSE();
		deltaSample = (int)(ldrattn.dataR[rVolume].i_SE & 32767) - sample;
		deltaPW = deltaSample > 0 ? -1 : 1;
		if(abs(deltaSample)>2){
			ldrattn.dataR[rVolume].pw_SE += deltaPW;
			setRSE(ldrattn.dataR[rVolume].pw_SE);
		}

		LOG_TRACE(LOG_INFO," rse DAC:%d ADC:%d ", ldrattn.dataR[rVolume].pw_SE,sample);
#endif
#if 0
		// LSH
		sample = getILSH();
		deltaSample = (int)(dataL[volume - 2].i_SH & 32767) - sample;
		deltaPW = deltaSample > 0 ? -1 : 1;
#if 0
		if (deltaPW * deltaSample > abs(deltaPW * getDeltaI(dataL[volume - 2].i_SH) - deltaSample) && dataL[volume - 2].pw_SH > 0 && dataL[volume - 2].pw_SH < 65535) {
			dataL[volume - 2].pw_SH += deltaPW;
			setLSH(dataL[volume - 2].pw_SH);
		}
#endif
		if(abs(deltaSample)>2){
			dataL[volume - 2].pw_SH += deltaPW;
			setLSH(dataL[volume - 2].pw_SH);
		}

		printf(" lsh DAC:%ld ADC:%d ",dataL[volume - 2].pw_SH,sample);

#endif
#if 1
		// RSH
		sample = getIRSH();
		deltaSample = (int)(ldrattn.dataR[rVolume].i_SH & 32767) - sample;
		deltaPW = deltaSample > 0 ? -1 : 1;
		if(abs(deltaSample)>2){
			ldrattn.dataR[rVolume].pw_SH += deltaPW;
			setRSH(ldrattn.dataR[rVolume].pw_SH);
		}

		LOG_TRACE(LOG_INFO," rsh DAC:%d ADC:%d\n",ldrattn.dataR[rVolume].pw_SH,sample);

#endif
		

	}
}



static int doSelfTest()
{
	unsigned long val;
	//switch calibration relays on to measure LDRs
	setCalibrationRelays(ON);

	// test if min/max resistance values are within limits
	// test series LDRs max val
	setLSE(0); setRSE(0);
	setLSH(65535); setRSH(65535); //set shunt R to minimum
	sleep(1);
#if 0
	val = getRLSE();
	if (val < LDR_ZERO_MIN) {
		return 1;
	}
#endif
	val = getRRSE();
		printf("---- RSE va is %d  \n",val);
	if (val < LDR_ZERO_MIN) {
		printf("---- RSE va is %d \n",val);
		return 1;
	}

	// test shunt LDRs max val
	setLSE(65535); setRSE(65535); //set series R to minimum
	setLSH(0); setRSH(0);
	sleep(1);
#if 0
	val = getRLSH();
	if (val < LDR_ZERO_MIN) {
		return 1;
	}
#endif
	val = getRRSH();
		printf(" RSH va is %d \n",val);
	if (val < LDR_ZERO_MIN) {
		printf(" RSH va is %d \n",val);
		return 1;
	}

	return 0;
}

void setMute()
{

	LOG_TRACE(LOG_INFO,"Volume set to Mute ------------\n ");
	setLSE(0); setRSE(0); setLSH(65535); setRSH(65535);
	sleep(1);		
}


void setVolume(int volume)
{
	unsigned char i = volume;

	setLSE(ldrattn.dataL[i].pw_SE); setRSE(ldrattn.dataR[i].pw_SE);
	setLSH(ldrattn.dataL[i].pw_SH); setRSH(ldrattn.dataR[i].pw_SH);
	LOG_TRACE(LOG_INFO,"Volume set to RSE %d RSH %d volume %d\n ",ldrattn.dataR[i].pw_SE,ldrattn.dataR[i].pw_SH,volume);

	sleep(1);		
}

int saveCalibration()
{
	char csvfile[MAXLEN];
	
	getCSVFilename(ldrattn_t.potImpedence,csvfile);
	if( writeCalibData(csvfile,ldrattn_t,1) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	} 
	return EXIT_SUCCESS;

}


int  initWiringPiDevices()
{

	wiringPiSetup();

	if(!ltc2655Setup(DAC_I2C_ADDR)) {
		return EXIT_FAILURE;
	} 
	writeCommand(CHANNEL_ALL,LTC2655_CMD_EXTERNAL_REF);

	if(!ads1115Setup(ADC1_PIN_BASE,ADC1_I2C_ADDR)) { 
		return EXIT_FAILURE;
	}

	if(!ads1115Setup(ADC2_PIN_BASE,ADC2_I2C_ADDR)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

int testSetup()
{

	unsigned long val;

	setLSE(0); setRSE(0);
	setLSH(65535); setRSH(65535); //set shunt R to minimum
	sleep(1);
#if 0
	val = getRLSE();
	if (val < LDR_ZERO_MIN) {
		return 1;
	}
#endif
	val = getIRSE();
	printf("----  IRSE val is %d \n",val);
	val = getIRSH();
	printf("----  IRSH val is %d \n",val);

	// test shunt LDRs max val
	setLSE(65535); setRSE(65535); //set series R to minimum
	setLSH(0); setRSH(0);
	sleep(1);
#if 0
	val = getRLSH();
	if (val < LDR_ZERO_MIN) {
		return 1;
	}
#endif
	val = getIRSE();
	printf(" IRSE val is %d \n",val);
	val = getIRSH();
	printf(" IRSH val is %d \n",val);

	return 1;


}

void storeMaxVolume(impedance,numsteps)
{
	ldrattn_t.dataL[numsteps].pw_SE = 0;
	ldrattn_t.dataR[numsteps].pw_SE = 0;
	ldrattn_t.dataL[numsteps].pw_SH = 65535;
	ldrattn_t.dataR[numsteps].pw_SH = 65535;
	ldrattn_t.targetres[numsteps].series = impedance*1000;
	ldrattn_t.targetres[numsteps].shunt = 0;
	
}
	
void storeZeroVolume(unsigned int impedance)
{
	ldrattn_t.dataL[0].pw_SE = 65535;
	ldrattn_t.dataR[0].pw_SE = 65535;
	ldrattn_t.dataL[0].pw_SH = 0;
	ldrattn_t.dataR[0].pw_SH = 0;
	ldrattn_t.targetres[0].series = 0;
	ldrattn_t.targetres[0].shunt = impedance*1000;

}


void * doCalibration(struct calibattr *calibdata) {
	unsigned int impedance;
	unsigned int numsteps;
	unsigned int temperature;
	int fd;

	
	//** local vars
	float att;
	unsigned long LSE_target, LSH_target, RSE_target, RSH_target, targetL0, targetR0;
	short int pwL0, pwL1, pwR0, pwR1 = 0;
	int iL0, iL1, iR0, iR1 = 0;
	unsigned long rL0, rL1, rR0, rR1 = 0;
	short int rangeL, rangeR;
	short int dpw, drm;
	unsigned int foundL, foundR = FALSE;
	unsigned int reloopL, reloopR = FALSE;
	unsigned int nojumpL, nojumpR = FALSE;
	unsigned int step = 0; 
	int percent = 0;
	
	unsigned long LSH_Min_Hi = 0 ;
	unsigned long RSH_Min_Hi = 0 ;
	unsigned long LSE_Min_Hi = 0 ;
	unsigned long RSE_Min_Hi = 0 ;

	char ipcdata = 'A';
	impedance = calibdata->impedance;	
	numsteps = calibdata->numsteps;
	temperature = calibdata->temperature;	
	fd = calibdata->pipefd;
	calibdata->status = CALIB_ONGOING;
	calibdata->sestep = 0;
	calibdata->shstep = 0;

	LOG_TRACE(LOG_INFO,"HERE %s:%d fd %d  %d  %d \n", __func__, __LINE__,fd,impedance,numsteps);

	int errc = doSelfTest();
	if (errc != 0) {
		write(fd,&ipcdata,1);	
		calibdata->status = CALIB_FAILED; 
		debugLog(LOG_INFO,"Calibration self test failed\n");
		return (void *)errc;
	}


	setCalibrationRelays(ON);

	//** get minimum R in HIGH R range
	setLSE(65535); setRSE(65535); //set series R to High range, minimum value
	setLSH(65535); setRSH(65535); //set shunt R to minimum

	LSE_Min_Hi = getRLSE();
	RSE_Min_Hi = getRRSE();
	setLSE(65535); setRSE(65535); //set series R to minimum
	setLSH(65535); setRSH(65535); //set shunt R to High range, almost minimum value

	LSH_Min_Hi = getRLSH();
	RSH_Min_Hi = getRRSH();
	setLSE(0); setLSH(0); setRSE(0); setRSH(0);

	rL0 = rR0 = 4000000;
	pwL0 = pwR0 = 0; pwL1 = pwR1 = 18600;
	iL0 = iR0 = 0;

	setLSE(0); setRSE(0);
	setLSH(65535); setRSH(65535);

	memset(&ldrattn_t,0,sizeof(ldrattn_t));
	ldrattn_t.potImpedence = impedance;
	ldrattn_t.calibSteps = numsteps;

	//loop for each volume sample needed
	int fp = open(LOCK_FILE, O_CREAT);
	if(fp == -1) {
		LOG_TRACE(LOG_INFO,"unable  to create the lock file");
	} else  {
		close(fp); 	
	}	
	debugLog(LOG_INFO,"Calibration started\n");
		
#if 1
	for (step = 1 ; step < (numsteps) ; step++) { //step=0 is mute step=calib=max
		
		if(calibStop) {
			calibStop = FALSE;
			pthread_exit(0);

		}
		int dbstep = numsteps-step; //db value for the step
		
		calibdata->status = CALIB_ONGOING;
		calibdata->sestep = step;
		float dbperstep = (float)DBMAX/(numsteps-1);

		//** calculate attenuation needed
		att = getAttFromStep((float)(dbperstep*dbstep));
		printf(" att %f dbstep %d dbperstep %f \n",att,dbstep, dbperstep);

		//** calculate target R values
		LSE_target = RSE_target = getRxFromAttAndImp(att, impedance*1000);
		LSH_target = RSH_target = getRyFromAttAndImp(att, impedance*1000);
		LSH_target = LOAD_IMPEDANCE * LSH_target / (LOAD_IMPEDANCE - LSH_target);
		RSH_target = LOAD_IMPEDANCE * RSH_target / (LOAD_IMPEDANCE - RSH_target);
		
		ldrattn_t.targetres[step].series = RSE_target;
		ldrattn_t.targetres[step].shunt= RSH_target;

		printf("Step:%d DB:%f Attn:%f LSE_target:%d LSH_target:%d RSE_target:%d RSH_target:%d \n", step, (dbperstep * dbstep), att, LSE_target, LSH_target, RSE_target, RSH_target);
		//** find LDR current for the target resistances calculated above
		foundL = foundR = false;
#if 1
		foundR = false;
                foundL = true;

#endif
		do {
			reloopL = reloopR = FALSE;  //reloop = if the jump was too large, loop again with step=1		
			if(calibStop) {
				calibStop = FALSE;
				pthread_exit(0);

			}


			setLSE(pwL1); setRSE(pwR1);

			//int del = getDelayHi(max(getILSE(), getIRSE())) * 100 - 500; // +variable delay
			printf(" Sett ing pwR0:%ld pwR1:%ld \n", pwR0,pwR1);
			//sleep(10);
			if(RSE_target >=  impedance/2) {
				sleep(1);
			} else {
				sleep(3);
			}

			if (!foundL) {
				rL1 = getRLSE();
				iL1 = getILSE();
			}
			if (!foundR) {
				rR1 = getRRSE();
				iR1 = getIRSE();
			}

			LOG_TRACE(LOG_INFO,"rL0:%d rL1:%d  rR1 %d iR1 %d\n", rL0, rL1, rR1, iR1);
			// test if found L
			if (rL0 >= LSE_target && LSE_target >= rL1 && !foundL) {
				if (pwL1 - pwL0 > 1) {
					pwL1 = pwL0 + 1;
					reloopL = TRUE; // found, but the PWM jump was too large, repeat with jump = 1
				}
				else {
					foundL = TRUE;
					if (rL0 - LSE_target >= LSE_target - rL1) {
						ldrattn_t.dataL[step].pw_SE = pwL1;
						ldrattn_t.dataL[step].i_SE = iL1;
					}
					else
					{
						ldrattn_t.dataL[step].pw_SE = pwL0;
						ldrattn_t.dataL[step].i_SE = iL0;
					}
					//printf("\n Got values left dac :%ld for R:%ld", dataL[i].pw_SE, LSE_target);
				}
			}

			if (!foundL && !reloopL)
			{
				if (pwL1 == MAX_LDR_INPUT) {
					write(fd,&ipcdata,1);	
					calibdata->status = CALIB_FAILED; 
					return 1;
				}

				if (rL0 < LSE_target && step > 1) { //go back
					pwL0--; pwL1--;
					rL0 = 4000000;
				}
				else
				{
					if (rL0 != rL1 && rL0 != 4000000)
						dpw = abs((rL1 - LSE_target) * (pwL1 - pwL0) / (rL0 - rL1));
					else
						dpw = 1;
					if (dpw == 0) dpw = 1;
					pwL0 = pwL1;
					pwL1 += dpw;


					iL0 = iL1;
					rL0 = rL1;
				}
			}

			// test if found R
			if (rR0 >= RSE_target && RSE_target >= rR1 && !foundR) {
				if (pwR1 - pwR0 > 1) {
					pwR1 = pwR0 + 1;
					reloopR = TRUE;
				}
				else {
					foundR = TRUE;
					if (rR0 - RSE_target >= RSE_target - rR1) {
						ldrattn_t.dataR[step].pw_SE = pwR1;
						ldrattn_t.dataR[step].i_SE = iR1;
					}
					else
					{
						ldrattn_t.dataR[step].pw_SE = pwR0;
						ldrattn_t.dataR[step].i_SE = iR0;
					}
					//printf("\n Got values right dac :%ld for R:%ld\n", dataR[i].pw_SE, RSE_target);

			//		int c=0;
			//		for(c=0; c < 100; c++){
			//			printf("\n R:%ld Acutal:%ld i:%ld\n", RSE_target, getRRSE(), getIRSE());
			//			sleep(1);
			//		}
				}
			}

			if (!foundR && !reloopR)
			{
				if (pwR1 == MAX_LDR_INPUT) {
					write(fd,&ipcdata,1);	
					calibdata->status = CALIB_FAILED; 
					return 2;
				}

				if (rR0 < RSE_target && step > 1) { //go back
					pwR0--; pwR1--;
					rR0 = 4000000;
				}
				else
				{
					if (rR0 != rR1 && rR0 != 4000000)
						dpw = (rR1 - RSE_target) * (pwR1 - pwR0) / (rR0 - rR1);
					else
						dpw = 1;
					if (dpw == 0) dpw = 1;
					pwR0 = pwR1;
					pwR1 += dpw;

					iR0 = iR1;
					rR0 = rR1;

					printf("HERE %s:%d  %d %d  %d\n", __func__, __LINE__,pwR0,pwR1,dpw);
				}
			}

		} while (!foundL || !foundR);

		printf("Step:%02d DB:%f Attn:%f dataL.pw_SE:%ld dataL.i_SE:%ld for LSE_target:%04d LSH_target:%04d",  step, (dbperstep * dbstep), att , ldrattn_t.dataL[step].pw_SE,ldrattn_t.dataL[step].i_SE, LSE_target, LSH_target);
		printf(" --- dataR.pw_SE:%ld dataR.i_SE:%ld for RSE_target:%04d RSH_target:%04d\n", ldrattn_t.dataR[step].pw_SE,ldrattn_t.dataR[step].i_SE, RSE_target, RSH_target);

		debugLog(LOG_INFO, "RLSE %05d ILSE %05d RRSE %05d IRSE %05d TargetSE %05d TargetSH %05d\n ",ldrattn_t.dataL[step].pw_SE,ldrattn_t.dataL[step].i_SE,ldrattn_t.dataR[step].pw_SE,ldrattn_t.dataR[step].i_SE,RSE_target, RSH_target );
		debugLog(LOG_INFO, "RLSE %05d ILSE %05d ",ldrattn_t.dataL[step].pw_SE,ldrattn_t.dataL[step].i_SE);
		debugLog(LOG_INFO, "RRSE %05d IRSE %05d ",ldrattn_t.dataR[step].pw_SE,ldrattn_t.dataR[step].i_SE);
		debugLog(LOG_INFO, "TargetSE %05d TargetSH %05d\n",RSE_target, RSH_target);

	} //end for series
#endif
	//** search for shunt R current
	//shunt: start at min R and go up
	rL0 = rR0 = 0;

	pwL0 = 30000;
	pwR0 = 30000;
	pwL1 = 30000;
	pwR1 = 30000;
	iL0 = iR0 = 0;
	targetL0 = targetR0 = 0;
	rangeL = rangeR = LOW;
	setLSE(65535); setRSE(65535);
	setLSH(65535); setRSH(65535);

	//loop for each volume sample needed
	for (step = 1; step < (numsteps) ; step++) {

		if(calibStop) {
			calibStop = FALSE;
			pthread_exit(0);

		}

		//byte step = i+2;
		
		int dbstep = numsteps-step; //db value for the step

		calibdata->shstep = step;
		float  dbperstep =(float) DBMAX/(numsteps-1);

		//** calculate target R values
		att = getAttFromStep(dbperstep*dbstep);

		LSE_target = RSE_target = getRxFromAttAndImp(att, impedance*1000);
		LSH_target = RSH_target = getRyFromAttAndImp(att, impedance*1000);

		//** adjust shunt R for the parallel load impedance
		LSH_target = LOAD_IMPEDANCE * LSH_target / (LOAD_IMPEDANCE - LSH_target);
		RSH_target = LOAD_IMPEDANCE * RSH_target / (LOAD_IMPEDANCE - RSH_target);

		LOG_TRACE(LOG_INFO,"Loop for LSE_target:%d LSH_target:%d RSE_target:%d RSH_target:%d %ld \n", LSE_target, LSH_target, RSE_target, RSH_target,time(0) );

		//** find LDR current for the target resistances
		foundL = foundR = false;
		nojumpL = nojumpR = false;
		foundL = true;
#if 1
                foundR = false;
	        foundL = true;
                nojumpL = nojumpR = false;
#endif
		//switch range
		if (rangeL == LOW && (LSH_target > LSH_Min_Hi)) {
			rangeL = HIGH;
			setLSH(65535);
			rL0 = 0; iL0 = 0;
			//pwL0 = pwL1 = MAX_LDR_INPUT;
			pwL0 = 30000;
			pwL1 = 30000;	
			targetL0 = 0;
		}
		if (rangeR == LOW && (RSH_target > RSH_Min_Hi)) {
			rangeR = HIGH;
			setRSH(65535);
			rR0 = 0; iR0 = 0;
			//pwR0 = pwR1 = MAX_LDR_INPUT;
			pwR0 = 30000;
			pwR1 = 30000;	
			targetR0 = 0;
		}

		// already found the value?
		if (targetL0 == LSH_target) {
			ldrattn_t.dataL[step].pw_SH = ldrattn_t.dataL[step - 1].pw_SH;
			ldrattn_t.dataL[step].i_SH = ldrattn_t.dataL[step - 1].i_SH;
			foundL = true;
		}
		if (targetR0 == RSH_target) {
			ldrattn_t.dataR[step].pw_SH = ldrattn_t.dataR[step - 1].pw_SH;
			ldrattn_t.dataR[step].i_SH = ldrattn_t.dataR[step - 1].i_SH;
			//PRINT("> Copy RSH i="); PRINT(dataL[i].i_SH); PRINTLN();
			foundR = true;
		}

		// search
		do {
			reloopL = reloopR = false;  //reloop = if the jump was too large, loop again with step=1
			if(calibStop) {
				calibStop = FALSE;
				pthread_exit(0);

			}

			setLSH(pwL1); setRSH(pwR1);

			//if (rangeL == HIGH || rangeR == HIGH) {
			//	int del = getDelayHi(max(getILSH(), getIRSH())) * 100 - 500;
			//}

			if (!foundL) {
				rL1 = getRLSH();
				iL1 = getILSH();
			}
			if (!foundR) {
				rR1 = getRRSH();
				iR1 = getIRSH();
			}

	
			LOG_TRACE(LOG_INFO,"shunt Loop for step:%d pwL1:%d pwR1:%d pwR0 %d rR1 %d iR1 %d RSH_target %d\n", step, pwL1, pwR1,pwR0,rR1,iR1,RSH_target);
			// test if found L
			if (rL0 <= LSH_target && LSH_target <= rL1 && !foundL) {
				if (pwL0 - pwL1 > 1) {
					pwL1 = pwL0 - 1;
					nojumpL = reloopL = true;
				}
				else {
					foundL = true;
					if (LSH_target - rL0 < rL1 - LSH_target) {
						ldrattn_t.dataL[step].pw_SH = pwL0;
						ldrattn_t.dataL[step].i_SH = iL0;
					}
					else {
						ldrattn_t.dataL[step].pw_SH = pwL1;
						ldrattn_t.dataL[step].i_SH = iL1;
					}
					targetL0 = LSH_target;
					if (rangeL == LOW)
						ldrattn_t.dataL[step].i_SH= ldrattn_t.dataL[step].i_SH | 32768; //bit15 == 1 means LOW R range
				}
			}

			if (!foundL && !reloopL) {
				if (pwL1 == 0) {
					write(fd,&ipcdata,1);	
					calibdata->status = CALIB_FAILED; 
					return 3;
				}

				else if (rL0 > LSH_target && pwL0 < MAX_LDR_INPUT) { //go back
					pwL0++; pwL1++;
					rL0 = 0;
				}

				else {
					if (pwL0 - pwL1 == 1 && rL1 - rL0 > 2 && !nojumpL) {
						if (rL0 != rL1 && rL0 != 0)
							dpw = abs((LSH_target - rL1) / (rL1 - rL0) / 2);
						else
							dpw = 1;
						if (dpw == 0) dpw = 1;
					}
					else dpw = 3;
					pwL0 = pwL1;
					pwL1 -= dpw;

					iL0 = iL1;
					rL0 = rL1;
				}
			}


			// test if found R
			if (rR0 <= RSH_target && RSH_target <= rR1 && !foundR)  {
				LOG_TRACE(LOG_INFO,"HERE %s:%d  %d %d %d %d %d\n", __func__, __LINE__,rR0,rR1,pwR0,pwR1,RSH_target);
				if (pwR0 - pwR1 > 1) {
					pwR1 = pwR0 - 1;
					nojumpR = reloopR = true;
				}
				else {
					foundR = true;
					//if (RSH_target - rR0 < rR1 - RSH_target) //{
					if (RSH_target - rR0 < rR1 - RSH_target) {
						ldrattn_t.dataR[step].pw_SH = pwR0;
						ldrattn_t.dataR[step].i_SH = iR0;
					}
					else {
						ldrattn_t.dataR[step].pw_SH = pwR1;
						ldrattn_t.dataR[step].i_SH = iR1;
					}
					targetR0 = RSH_target;
					if (rangeR == LOW)
						ldrattn_t.dataR[step].i_SH = ldrattn_t.dataR[step].i_SH | 32768;
				}
			}

			if (!foundR && !reloopR) {
				if (pwR1 == 0) {
					write(fd,&ipcdata,1);	
					calibdata->status = CALIB_FAILED; 
					return 4;
				}

				else if (rR0 > RSH_target && pwR0 < MAX_LDR_INPUT) { //go back
					pwR0++; pwR1++;
					rR0 = 0;
				}

				else
				{
					LOG_TRACE(LOG_INFO,"HERE %s:%d %d %d %d %d \n", __func__, __LINE__,pwR0,pwR1,rR1,rR0);
					if (pwR0 - pwR1 == 1 && rR1 - rR0 > 2 && !nojumpR)  {
						if (rR0 != rR1 && rR0 != 0)  {
							dpw = abs((RSH_target - rR1) / (rR1 - rR0) / 2);
						}
						else{
							dpw = 1;
						}
						if (dpw == 0) dpw = 1;
					}
					else { 
						if(RSH_target < 2048)
							dpw = 20;
						else 
							dpw = 1;
					}
					pwR0 = pwR1;
					pwR1 -= dpw;


					iR0 = iR1;
					rR0 = rR1;
					LOG_TRACE(LOG_INFO,"HERE %s:%d pwR0  %d pwR1  %d dpw %d\n", __func__, __LINE__,pwR0,pwR1,dpw);
				}
			}

		} while (!foundL || !foundR);
		printf("Step:%02d DB:%f Attn:%f dataL.pw_SH:%ld dataL.i_SH:%ld for LSE_target:%04d LSH_target:%04d time%ld ", step, (dbperstep * dbstep), att , ldrattn_t.dataL[step].pw_SH,ldrattn_t.dataL[step].i_SH, LSE_target, LSH_target,time(0));
		printf("Step:%f DB:%f Attn:%f dataR.pw_SH:%ld dataR.i_SH:%ld for RSE_target:%04d RSH_target:%04d\n",step, (dbperstep * dbstep), att , ldrattn_t.dataR[step].pw_SH,ldrattn_t.dataR[step].i_SH,  RSE_target, RSH_target);

//		debugLog(LOG_INFO, "step :%02d DB:%02d Attn:%f RLSH %ld ILSH %ld RRSH %ld IRSH %ld TargetSE %ld TargetSH %ld\n",i,db, att , ldrattn_t.dataL[i].pw_SH,ldrattn_t.dataL[i].i_SH,ldrattn_t.dataR[i].pw_SH,ldrattn_t.dataR[i].i_SH,RSE_target, RSH_target);

		debugLog(LOG_INFO, "RLSH %05d ILSH %05d ",ldrattn_t.dataL[step].pw_SH,ldrattn_t.dataL[step].i_SH);
		debugLog(LOG_INFO, "RRSH %05d IRSH %05d ",ldrattn_t.dataR[step].pw_SH,ldrattn_t.dataR[step].i_SH);
		debugLog(LOG_INFO, " TargetSE %05d TargetSH %05d\n",RSE_target, RSH_target);

	} //end for shunt

	storeZeroVolume(impedance);	
	storeMaxVolume(impedance,numsteps);	

	if(fp != -1) {
		unlink(LOCK_FILE);
	}

	debugLog(LOG_INFO,"Calibration completed\n");
	write(fd,&ipcdata,1);	
	calibdata->status = CALIB_SUCCESS; 
	return 0;
}
