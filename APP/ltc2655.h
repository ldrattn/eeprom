#ifndef LTC2655_H
#define LTC2655_H 

#define CHANNEL_ALL     (0x0F)
#define CHANNEL_1       (0)
#define CHANNEL_2       (1)
#define CHANNEL_3       (2)
#define CHANNEL_4       (3)

/* Commands to LTC2655 */
#define LTC2655_CMD_WR_INPUT_N          0x00 /* Write only register n */
#define LTC2655_CMD_UP_N                0x01 /* Update only register n */
#define LTC2655_CMD_WR_UP_ALL           0x02 /* Write & Update register n*/
#define LTC2655_CMD_WR_UP_N             0x03 /* Write & Update register n*/
#define LTC2655_CMD_PD_N                0x04 /* Power Down   n */
#define LTC2655_CMD_PD_ALL              0x05 /* Power Down all DAC */
#define LTC2655_CMD_PD_ALL              0x05 /* Power Down all DAC */
#define LTC2655_CMD_INTERNAL_REF        0x06 /* Select Internal Reference(Power-Up Reference)*/
#define LTC2655_CMD_EXTERNAL_REF        0x07 /* Select External Reference(Power-Up Reference)*/

#define TRUE 1
#define FALSE 0

int ltc2655Setup (int i2cAddr) ;
void writeCommand (int channel, int command);
int writeDigitalData (int channel, int val);
#endif
