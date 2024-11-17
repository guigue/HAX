#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "transform.h"

void main(){

  unsigned short i,j;
  FILE *fp, *fpout, *fpoutb;
  char a, b;
  char ulSample[9], ulTimestamp_sec[9], usTimestamp_ms[5],u64husec[17],rawGolay[9], adc_str[7],rawPowerSupply[9];
  unsigned long int N, sec;
  unsigned int ms;
  unsigned husec;
  int adc, PS;
  struct Data {
    unsigned long int N, sec;
    unsigned int ms;
    unsigned husec;
    int adc, PS;
  } data; 

  
  if ((NULL != (fp = fopen(ARCHIVEIN,"r"))) &
      (NULL != (fpout=fopen(ARCHIVEOUT,"w"))) &
      (NULL != (fpoutb=fopen(ARCHIVEBOUT,"wb"))) )
    {
      
      printf("\n\n");
      printf("ulSample  ulTimestamp_sec  usTimestamp  u64husec  rawGolay  rawPowerSupply\n");
      printf("--------------------------------------------------------------------------\n");
      
      //for(i=1;i<=20;i++)
      while(1)
	{
	  for (j=1;j<5;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&ulSample[7-2*(j-1)],&b,1);
	    memcpy(&ulSample[6-2*(j-1)],&a,1);
	  }
	  data.N = strtol(ulSample,NULL,16);
	  

	  for (j=1;j<5;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&ulTimestamp_sec[7-2*(j-1)],&b,1);
	    memcpy(&ulTimestamp_sec[6-2*(j-1)],&a,1);
	  }
	  data.sec = strtol(ulTimestamp_sec,NULL,16);

	  for (j=1;j<3;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&usTimestamp_ms[3-2*(j-1)],&b,1);
	    memcpy(&usTimestamp_ms[2-2*(j-1)],&a,1);
	  }
	  data.ms = strtol(usTimestamp_ms,NULL,16);
	
	  for (j=1;j<9;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&u64husec[15-2*(j-1)],&b,1);
	    memcpy(&u64husec[14-2*(j-1)],&a,1);
	  }
	  data.husec = strtol(u64husec,NULL,16);

	  for (j=1;j<5;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&rawGolay[7-2*(j-1)],&b,1);
	    memcpy(&rawGolay[6-2*(j-1)],&a,1);
	  }
	  memcpy(adc_str,&rawGolay[2],6);
	  adc = strtol(adc_str,NULL,16);
	  data.adc = strtol(rawGolay,NULL,16);
	  
	  for (j=1;j<5;j++){
	    a = fgetc(fp);
	    b = fgetc(fp);
	    memcpy(&rawPowerSupply[7-2*(j-1)],&b,1);
	    memcpy(&rawPowerSupply[6-2*(j-1)],&a,1);
	  }
	  data.PS = strtol(rawPowerSupply,NULL,16);
	
	  if (feof(fp)){
	    break ;
	  }

	  printf(" %8u  %14u  %11d  %8lu  %7d  %14d\n", data.N, data.sec, data.ms, data.husec, adc, data.PS);
	  fprintf(fpout,"%d %d\n",data.husec,adc);
	  fwrite(&data,sizeof(data),(size_t) 1, fpoutb);
	  
	}
      
      fclose(fp);
      fclose(fpout);
      fclose(fpoutb);
    }
    
  return;
}
  

