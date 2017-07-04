//#include <linux/types.h>
//#include <linux/ioctl.h>
//#include <linux/magic.h>

#ifndef ACTION_H
#define ACTION_H 

#define LDR_LOW_TOLERANCE 2 // percent between theory and measured R value acceptable for low R values
#define LDR_ZERO_MIN 40// minimum acceptable R when PWM=0 
#define LDR_FULL_MAX 200    // maximum acceptable R when PWM=255. 

//#define TRUE 1
//#define FALSE 0
//#ifndef TRUE
//#  define       TRUE    (1==1)
//#  define       FALSE   (1==2)
//#endif
//
#define ON 1
#define OFF  0

#define HIGH 1
#define LOW 0
#define MUTE 0

#define PIN_DAC_LSE 0 
#define PIN_DAC_LSH 1
#define PIN_DAC_RSE 2
#define PIN_DAC_RSH 3

#define PIN_ADC_LSE 4 
#define PIN_ADC_LSH 5
#define PIN_ADC_RSE 6
#define PIN_ADC_RSH 7
#define PIN_CALIB_L 8 
#define PIN_CALIB_R 9
/******* Attenuator control *******/
#define NOM_IMPEDANCE 30000 //** target nominal attenuator impedance, between 5000 and 50000 Ohm. Recommended: 10000. Higher = less precision
#define LOAD_IMPEDANCE 220000 //** input impedance of the amplifier that follows the attenuator. Should not be less than 100K. Default: 220000 ohm
//#define VOL_MAX_STEP 3     //** maximum volume steps; range: 20...80. Higher = more memory usage
#define VOL_DEFAULT 25      //** default volume step. Must be <= than MAX

/******* LDR measured values *******/
#define LDR_VOLTAGE 5.0     //** precisely measured value of the +5V supply (with decimal point. Default: 5.0)
#define LDR_R1 30000        //** precisely measured value of R1 resistor (default: 10000 ohm)
#define LDR_R18 30000       //** precisely measured value of R18 resistor (default: 10000 ohm)

// if any measured value is > 200 ohm, replace the LDR. Normal values are around 100 ohm.
#define LDR_LSE_MIN 100     //** measured value of left series LDR R at maximum current
#define LDR_LSH_MIN 100 //** measured value of left shunt LDR R at maximum current
#define LDR_RSE_MIN 60 //** measured value of right series LDR R at maximum current
#define LDR_RSH_MIN 60 //** measured value of right shunt LDR R at maximum current

/*
#define LDR_IOCTL_IDENT               'l'

#define LDR_IOCTL_WR_CHAN1\
        _IOW(LDR_IOCTL_IDENT, 0x80, __u32)
#define LDR_IOCTL_WR_CHAN2\
        _IOW(LDR_IOCTL_IDENT, 0x81, __u32)
#define LDR_IOCTL_WR_CHAN3\
        _IOW(LDR_IOCTL_IDENT, 0x82, __u32)
#define LDR_IOCTL_WR_CHAN4\
        _IOW(LDR_IOCTL_IDENT, 0x83, __u32)
#define LDR_IOCTL_RD_CHAN1\
        _IOR(LDR_IOCTL_IDENT, 0x84, __u32)
#define LDR_IOCTL_RD_CHAN2\
        _IOR(LDR_IOCTL_IDENT, 0x85, __u32)
#define LDR_IOCTL_RD_CHAN3\
        _IOR(LDR_IOCTL_IDENT, 0x86, __u32)
#define LDR_IOCTL_RD_CHAN4\
        _IOR(LDR_IOCTL_IDENT, 0x87, __u32)
#define LDR_IOCTL_RD_CHAN5\
        _IOR(LDR_IOCTL_IDENT, 0x88, __u32)
#define LDR_IOCTL_RD_CHAN6\
        _IOR(LDR_IOCTL_IDENT, 0x89, __u32)

*/

#define DAC_I2C_ADDR 0x10 
#define DAC_CHAN1 0
#define DAC_CHAN2 1
#define DAC_CHAN3 2
#define DAC_CHAN4 3

#define ADC1_PIN_BASE 100
#define ADC2_PIN_BASE 200
#define ADC1_I2C_ADDR 0x48 
#define ADC2_I2C_ADDR 0x49 

#define ADC_CHAN1 (ADC1_PIN_BASE + 0)
#define ADC_CHAN2 (ADC1_PIN_BASE + 1) 
#define ADC_CHAN3 (ADC1_PIN_BASE + 2)
#define ADC_CHAN4 (ADC1_PIN_BASE + 3)
#define ADC_CHAN5 (ADC1_PIN_BASE + 0)
#define ADC_CHAN6 (ADC1_PIN_BASE + 1)
#define PGA_4V 1
#define DATA_RATE_475 6 

#define DELAY_LDR_STABLE	1
#define MAX_LDR_INPUT	65535 

#define LOCK_FILE "/var/lock/ldrlock" 

#endif
