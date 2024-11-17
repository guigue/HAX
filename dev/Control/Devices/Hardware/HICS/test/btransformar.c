#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#define ARCHIVO 'hics-hex-seq.txt'

void main(){

  unsigned short i,j;
  FILE *fp, *fpout;
  char a, b;
  char ulSample[9], ulTimestamp_sec[9], usTimestamp_ms[5],u64husec[17],rawGolay[7], rawPowerSupply[9];
  unsigned long int N, sec;
  unsigned int ms;
  unsigned husec;
  int adc, PS; 

  
  if ((NULL != (fp = fopen("hics-hex-seq.txt","r"))) & (NULL != (fpout=fopen("hics-ascii","w"))))
    {
      
      printf("\n\n");
      printf("ulSample  ulTimestamp_sec  usTimestamp  u64husec rawGolay rawPowerSupply\n");
      printf("------------------------------------------------------------------------\n");
      
      //      for(i=1;i<=5;i++)
      while(1)
