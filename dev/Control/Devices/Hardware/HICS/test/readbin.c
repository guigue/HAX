#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "data_transfer.h"

#define TEST_DATA_FILE "hats-2021-08-26T1800.rbd"
#define OUTPUT_TEST_FILE "hats-2021-08-26T1800.txt"

unsigned long bin_size(FILE * fp)
{
  fseek(fp, 0L, SEEK_END);
  return ftell(fp);
}

void main(){

  FILE *fpi, *fpo;
  unsigned char buffer[26];
  int dm_lngth =  26, nrec, rec ;
  receive_data_str data;
  int adc, ps;
  unsigned long int N, sec;
  unsigned short ms;
  unsigned husec;
  
  if (NULL != (fpi = fopen(TEST_DATA_FILE,"rb")))
    { 
     nrec = bin_size(fpi) / dm_lngth;
     printf("N Rec = %d\n\n",nrec);

     fpo = fopen(OUTPUT_TEST_FILE,"w");
     
     fseek(fpi,0L,SEEK_SET);
     while ( 0< fread(buffer, dm_lngth, 1,fpi) ){

       memcpy(&N,buffer,4);
       memcpy(&sec,buffer+4,4);
       memcpy(&ms,buffer+8,2);
       memcpy(&husec,buffer+10,8);
       
       memcpy(&adc,buffer+18,4);
       adc = adc & 0x00FFFFFF;
       if ((adc & 0x0800000)>0) adc-=0x1000000;

       memcpy(&ps,buffer+22,4);
       ps = ps & 0x00FFFFFF;
       if ((ps & 0x0800000)>0) ps-=0x1000000;

       fprintf(fpo,"%d, %d, %d, %ld, %d, %d\n",N,sec,ms,husec,adc,ps);
     }
     fclose(fpi);
     fclose(fpo);
    }
    
  return;
}
  

