/*

   gcc -g -I../inc ../FFT/src/windft/windowed_dft.c readbin2.c -lm -lfftw3 -o readbin2

*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "data_transfer.h"
#include <complex.h>

#include "windowed_dft.h"

#define TEST_DATA_FILE "hats-2021-08-26T1700.rbd"
#define OUTPUT_TEST_FILE "hats-2021-08-26T1700.csv"

#define TARGET_FREQUENCY 20 // frequency that we are interested 
#define WINDOW_SIZE 128     // number of sample point that compose each DFT analysis
#define STEPS 64           // number of "walked" sample points for each window
#define SAMPLING_INTERVAL 0.001
#define	SAMPLING_FREQUENCY 1/SAMPLING_INTERVAL
#define SIGNAL_LENGTH MAX_RECORDS_TO_BLOCK
#define NREAD 64 

void fft(double *, unsigned long * , unsigned long *, double *) ;

unsigned long bin_size(FILE * fp)
{
  fseek(fp, 0L, SEEK_END);
  return ftell(fp);

}


int num_windows = floor((SIGNAL_LENGTH - WINDOW_SIZE + 1) / STEPS);


int main(){
  
  struct timeval time_init,time_end;
  clock_t begin, end, max=0, min=100000, Dtime;
  FILE *fpi, *fpo, *fpo2;

  int adc, ps, i, j, dm_lngth = DATA_QUARK_SIZE*MAX_RECORDS_TO_BLOCK, nrec;
  unsigned short ms;  
  unsigned long int N, sec, husec, husec2[MAX_RECORDS_TO_BLOCK], m_husec[num_windows];
  double signal[MAX_RECORDS_TO_BLOCK], * fftCoeff = malloc(sizeof(double)*num_windows);
  unsigned char buffer[dm_lngth];
  receive_data_str data[MAX_RECORDS_TO_BLOCK];
  
  if (NULL != (fpi = fopen(TEST_DATA_FILE,"rb")))
    {
      fpo = fopen(OUTPUT_TEST_FILE ,"w");
      fprintf(fpo,"Amp,Husec\n");

      fpo2 = fopen("data.csv" ,"w");
      fprintf(fpo2,"Husec,ADC\n");
      
      nrec = bin_size(fpi) / DATA_QUARK_SIZE ;
      printf("\n\nN Rec = %d N Windows = %d\n\n",nrec,num_windows);

      fseek(fpi,0L,SEEK_SET);
      while ( 0< fread(buffer, dm_lngth, 1,fpi) ){

	gettimeofday(&time_init,NULL);
	begin = clock();
	for (i=0;i<MAX_RECORDS_TO_BLOCK;i++){
	 
	  memcpy(&N,buffer+i*DATA_QUARK_SIZE,4);
	  memcpy(&sec,buffer+4+i*DATA_QUARK_SIZE,4);
	  memcpy(&ms,buffer+8+i*DATA_QUARK_SIZE,2);
	  memcpy(&husec,buffer+10+i*DATA_QUARK_SIZE,8);
       
	  memcpy(&adc,buffer+18+i*DATA_QUARK_SIZE,4);
	  adc = adc & 0x00FFFFFF;
	  if ((adc & 0x0800000)>0) adc-=0x1000000;

	  memcpy(&ps,buffer+22+i*DATA_QUARK_SIZE,4);
	  ps = ps & 0x00FFFFFF;
	  if ((ps & 0x0800000)>0) ps-=0x1000000;

	  data[i].ulSample = N;
	  data[i].ulTimestamp_sec = sec;
	  data[i].usTimestamp_ms = ms ;
	  data[i].u64husec = husec;
	  data[i].rawGolay = adc;
	  data[i].rawPS = ps;
	  signal[i] = (double) adc ;
	  husec2[i] = husec ;
	  fprintf(fpo2,"%ld,%d\n",husec,adc);
	}
	fft(signal, husec2, m_husec, fftCoeff) ;
	for (j=0;j<num_windows;j++) fprintf(fpo,"%f,%ld\n",*fftCoeff+j,*(m_husec+j));
		
	gettimeofday(&time_end,NULL);       
	end = clock();
	Dtime = 1E6*(end-begin)/CLOCKS_PER_SEC;
	if (Dtime < min) min=Dtime;
	if (Dtime > max) max=Dtime;
       
      }
      fclose(fpi);
      fclose(fpo);
      fclose(fpo2);
      printf("CPU time Max = %5.1f us Min = %5.1f us\n",(double)max, (double)min);

    }

  free(fftCoeff);
}

void fft(double * signal, unsigned long * husec, unsigned long * m_husec, double * fftCoeff){

  int i;
  for(i = 0; i < num_windows; i++)
    {
    fftCoeff[i] = goertzel_amplitude(&signal[i*STEPS]     ,
				     TARGET_FREQUENCY     ,
				     WINDOW_SIZE          ,
				     SAMPLING_FREQUENCY   ,
				     FLATTOP) / (WINDOW_SIZE)      ;
    m_husec[i] = husec[i*STEPS+WINDOW_SIZE/2];    
    }
  i+1;
}
