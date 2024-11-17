#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <errno.h>
#include "../windft/windowed_dft.h"

/* Example processing real world data from HATS and outputting to a file ONLY a single frequency amplitude using goertzel algorithm
or the FFTW whole spectrum approach.

Description: 
	This algorithm outputs the amplitude of a single frequency and how it changes over time. 

Programmed by Guillermo Giménez de Castro and Manuel Giménez de Castro in 2021.
*/

#define target_frequency 20 /*frequency that we are interested*/
#define window_size 256 /*number of sample point that compose each DFT analysis*/
#define steps 128 /*number of "walked" sample points for each window*/

int main(void){
	FILE *fdata; //File with rought signal data
	//FILE *ftarget_frequency_amp; 
	FILE *ftarget_frequency_amp_goertzel; //Output file with the amplitude change of the <target_frequency> frequency over time
	size_t len = 0 ;
	ssize_t Nread;
	char * buff = NULL, str[20], file_name[100];
	float sampling_interval, sampling_frequency;
	long signal_lenght, i=0, j=0, target_fs_i;
        const int RECTANGULAR=0, FLATTOP=1, CHEBYSHEV=2, HANN=3, TRIANGULAR=4;
	const int SAVE_AMP = 0, SAVE_AMP_AND_PHASE = 1;
	const int half = floor(window_size/2)+1; //number of frequencies that will be stored
	int num_windows; //number of windows of size <window_size> that fit in the signal of lenght <signal_lenght> with steps of 1 sample point
	int errsv; //to catch errors 
	double *signal, *time, *windowed_signal, *phase, *amp; 
	//double *target_frequency_amp; 
	double *target_frequency_amp_goertzel; //signal, time, filtered signal, phase, and amplitude arrays 
	complex *fft_out; 

	fdata = fopen("../../data/test01.txt","r"); //reads file with data
	//write on this file
	//snprintf(file_name, 50, "../../data/processed/%d_frequency_amp.txt",target_frequency);
	//ftarget_frequency_amp = fopen(file_name, "w");
	snprintf(file_name, 100, "../../data/processed/%d_frequency_amp_goertzel_steps_128.txt",target_frequency);
	ftarget_frequency_amp_goertzel = fopen(file_name, "w");

	Nread = getline(&buff, &len, fdata);
	sscanf(buff,"%5c%f",str,&sampling_interval);
	
	sampling_frequency = 1/sampling_interval;

	Nread = getline(&buff, &len, fdata);	
	sscanf(buff,"%4c%ld",str,&signal_lenght);

	num_windows = (int) floor((signal_lenght-window_size+1)/steps);

	//arrays that won't be stored on file
	fft_out = (complex *) malloc(sizeof(complex)*half);
	windowed_signal = (double *) malloc(sizeof(double)*window_size);
	phase = (double *) malloc(sizeof(double)*half);
	amp = (double *) malloc(sizeof(double)*half);

	//array that will be outputted
	//target_frequency_amp = (double *) malloc(sizeof(double)*num_windows);
	target_frequency_amp_goertzel = (double *) malloc(sizeof(double)*num_windows);

	printf("\nTargeting frequency %d with window size %d and %d steps\n signal_lenght = %ld and sampling_interval = %f\n\n", target_frequency, window_size, steps, signal_lenght,sampling_interval);

	Nread = getline(&buff, &len, fdata);
	signal = malloc(signal_lenght*sizeof(double));
	time = malloc(signal_lenght*sizeof(double));

	i = 0; //reset the counter for the while statement
	while ((Nread = getline(&buff, &len, fdata)) != -1){
		sscanf(buff,"%lf%lf",&signal[i],&time[i]);
		i++;
	}

	//here we will extract only the "around" 20 hz frequency. Depending of the window size and signal lenght we can't pick 20 hz exaclty
	target_fs_i = (int) floor(target_frequency*window_size/sampling_frequency);
	//applies the algorithm for every window
	for(i = 0; i < num_windows; i++){
		printf("Calculating dft of window %ld\r", i);
		//windowed_dft(&signal[i*steps], fft_out, windowed_signal, amp, phase, RECTANGULAR, SAVE_AMP, window_size);
		//target_frequency_amp[i] = amp[target_fs_i];
		target_frequency_amp_goertzel[i] = goertzel_amplitude(&signal[i*steps], target_frequency, window_size, sampling_frequency, FLATTOP); //goertzel algorithm to only calculate a single frequency amplitude
	}

	printf("\n");
	//fprintf(ftarget_frequency_amp, "target_frequency = %d\nreal_frequency = %f\nsampling_frequency = %f\nsignal_lenght = %d\n", target_frequency, sampling_frequency*target_fs_i/window_size ,sampling_frequency, window_size);
	fprintf(ftarget_frequency_amp_goertzel, "target_frequency = %d\nreal_frequency = %f\nsampling_frequency = %f\nsignal_lenght = %d\nsteps = %d\n", target_frequency, sampling_frequency*target_fs_i/window_size ,sampling_frequency, window_size, steps);
	for(i = 0; i < num_windows; i++){
		//fprintf(ftarget_frequency_amp, "%f ", target_frequency_amp[i]);
		fprintf(ftarget_frequency_amp_goertzel, "%f ", target_frequency_amp_goertzel[i]);
	}

	printf("\n");
	//closing file and freeing memory
	fclose(fdata);
	//fclose(ftarget_frequency_amp);
	fclose(ftarget_frequency_amp_goertzel);
	free(amp);
	free(fft_out);
	free(phase);
	free(windowed_signal);
	free(signal);
	free(time);
	//free(target_frequency_amp);
	free(target_frequency_amp_goertzel);
	
	return 0;
}
