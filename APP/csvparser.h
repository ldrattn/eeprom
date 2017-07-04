#ifndef CSVPARSER_H 
#define CSVPARSER_H

void getCSVFilename(unsigned int,char *);
int readCalibData(char *Fname); 
int writeCalibData(char *Fname,LDRAttr ldrattn_t,unsigned int flag);

#endif
