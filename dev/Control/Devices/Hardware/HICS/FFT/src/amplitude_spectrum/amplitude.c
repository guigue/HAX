#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <errno.h>
#include "../windft/windowed_dft.h"

/* Example processing real world data. This file outputs the spectrum of every window 

Description: 
	Here we will apply the windowed_dft to process real data. It outputs the amplitude over frequency in multiple files of each 
	window, max 1000 though.

Programmed by Guillermo Giménez de Castro and Manuel Giménez de Castro in 2021.*/

#define window_size 256
#define steps 1 /*number of "walked" sample points for each window*/

int main(void){
	FILE *fdata; //File with rought signal data
	FILE **files_amp; //Possibly multiple files will be needed. Depends on the data
	size_t len = 0 ;
	ssize_t Nread;
	char * buff = NULL, str[20], file_name[50];
	float sampling_interval, sampling_frequency;
	long signal_lenght,i=0, j=0;
        const int RECTANGULAR=0, FLATTOP=1, CHEBYSHEV=2, HANN=3, TRIANGULAR=4;
	const int SAVE_AMP = 0, SAVE_AMP_AND_PHASE = 1;
	const int half = floor(window_size/2)+1; //number of frequencies that will be stored
	int num_windows; //number of windows of size <window_size> fit in the signal of lenght <signal_lenght>
	int errsv;
	double *signal, *time, *windowed_signal, *phase; //signal and time arrays 
	complex *fft_out; 
	double **amp_arrays; //arrays with processed data 

	fdata = fopen("../../data/test01.txt","r");
	Nread = getline(&buff, &len, fdata);
	sscanf(buff,"%5c%f",str,&sampling_interval);
	
	sampling_frequency = 1/sampling_interval;

	Nread = getline(&buff, &len, fdata);	
	sscanf(buff,"%4c%ld",str,&signal_lenght);

	num_windows = (int) floor((signal_lenght-window_size+1)/steps);

	//this is done to avoid errno 24, which is too many open files error.
	if (num_windows > 1000){
		num_windows = 1000;
	} 

	files_amp = (FILE **) malloc(sizeof(FILE *)*num_windows);

	//arrays that won't be stored in files
	fft_out = (complex *) malloc(sizeof(complex)*half);
	windowed_signal = (double *) malloc(sizeof(double)*window_size);
	phase = (double *) malloc(sizeof(double)*half);
	
	//processed data array or arrays (or matrix, whatever you wish). This WILL be stored in files
	amp_arrays = (double **) malloc(sizeof(double*)*num_windows);

	for(i = 0; i < num_windows; i++){
		snprintf(file_name, 50, "../../data/rough/test01_amp_%ld.txt",i);
		files_amp[i] = fopen(file_name, "w"); //open every file (MAX 1024 though...)
		if (files_amp[i] != NULL){
			printf("Opening file %s\r", file_name);
		} else {
			errsv = errno;
			printf("Failed to open file %s with error number %d", file_name, errsv);
			return 0; //close
		}
		amp_arrays[i] = (double *) malloc(sizeof(double)*half);
	}

	printf("\n\n signal_lenght = %ld	sampling_interval = %f\n\n",signal_lenght,sampling_interval);

	Nread = getline(&buff, &len, fdata);
	signal = malloc(signal_lenght*sizeof(double));
	time = malloc(signal_lenght*sizeof(double));

	i = 0; //reset the counter for the while statement
	while ((Nread = getline(&buff, &len, fdata)) != -1){
		sscanf(buff,"%lf%lf",&signal[i],&time[i]);
		i++;
	}

	//applies the algorithm for every window
	for(i = 0; i < num_windows; i++){
		printf("Calculating dft of window %ld\r", i);
		windowed_dft(&signal[i*steps], fft_out, windowed_signal, amp_arrays[i], phase, FLATTOP, SAVE_AMP, window_size);
	}

	printf("\n");
	for(i = 0; i < num_windows; i++){
		printf("Writing file number %ld\r", i);
		fprintf(files_amp[i], "sampling_frequency = %f\nsignal_lenght = %d\n", sampling_frequency, window_size);
		for(j = 0; j < half; j++){
                        fprintf(files_amp[i], "%f ",amp_arrays[i][j]);
		}
	}

	printf("\n");
	//closing file and freeing memory
	fclose(fdata);
	for(i = 0; i < num_windows; i++){
		fclose(files_amp[i]);//close every file
		free(amp_arrays[i]); //free every amplitude array 
	}
	free(files_amp);
	free(fft_out);
	free(amp_arrays);
	free(phase);
	free(windowed_signal);
	free(signal);
	free(time);
	
	return 0;
}	
