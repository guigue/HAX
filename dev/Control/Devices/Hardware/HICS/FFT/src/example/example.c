#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include "../windft/windowed_dft.h"

/* Example of using Windowed DFT for HATS using FFTW. Based on Tiago Giorgetti's implementation in Matlab, "Influência das janelas 
de ponderação na obtenção da amplitude de sinais periódicos no domínio da frequência".

Description: 
	Here we will generate a signal and apply one of the filters on the whole set of data points. Then we will output
	4 files with the phase, amplitude, filtered signal, and the dft rough data.

Programmed by Manuel Giménez de Castro in 2021.
*/

int main(){
	double *mod_array, *arg_array, *signal, *t, *windowed_signal;
	complex *out;
	int signal_lenght, i, half;
	const int RECTANGULAR=0, FLATTOP=1, CHEBYSHEV=2, HANN=3, TRIANGULAR=4;
	double sampling_frequency, sampling_interval, df, FN1, FN2, f1, f2;
	//File outputs
	FILE *farg,*ffft,*fmod, *fsignal;
	// number of sample points
	signal_lenght = pow(2,14);
	// number of frequencies that will be stored
	half = (int)floor(signal_lenght/2)+1;
	// sampling frequency
	sampling_frequency = 1000;
	// Base Frequencies (?)
	FN1 = 328;
	FN2 = 984;
	// sampling interval
	sampling_interval = 1/sampling_frequency;
	// frequency increment
	df = 1/(signal_lenght*sampling_interval);
	// more frequency stuff I am not sure???
	f1 = FN1*df;
	f2 = FN2*df;
	// signal that will be decomposed in frequencies
	signal = (double*) malloc(sizeof(double)*signal_lenght);
	// array with the modulus of the dft. Because we are dealing with real valued time series we don't the same lenght of the signal to store the dft. We know that it will be symmetric with respect to the middle
	mod_array = (double*) malloc(sizeof(double)*half);
	//array with the argument of the dtt array
	arg_array = (double*) malloc(sizeof(double)*half);
	//time array
	t = (double*)malloc(sizeof(double)*signal_lenght);
	//array with the dft 
	out = (complex*) malloc(sizeof(complex)*half);
	//array with the windowed, or filtered, signal
	windowed_signal = (double *)malloc(sizeof(double)*signal_lenght);

	//will generate the time array and the signal
	for(i=0; i<signal_lenght; i++){
		t[i] = sampling_interval*i;
		//signal[i] = sin(2*M_PI*f1*t[i]) + 0.2*sin(2*M_PI*f2*t[i]);
		signal[i] = sin(2*M_PI*f1*t[i]); //Pure sine
	}
	
	farg = fopen("argument_array.txt","w");
	ffft = fopen("fft_array.txt","w");
	fmod = fopen("mod_array.txt","w");
	fsignal = fopen("signal_array.txt","w");
	
	fprintf(farg, "sampling_frequency = %f\nsignal_lenght = %d\n", sampling_frequency, signal_lenght);
	fprintf(ffft, "sampling_frequency = %f\nsignal_lenght = %d\n", sampling_frequency, signal_lenght);
	fprintf(fmod, "sampling_frequency = %f\nsignal_lenght = %d\n", sampling_frequency, signal_lenght);
	fprintf(fsignal, "sampling_frequency = %f\nsignal_lenght = %d\n",sampling_frequency,signal_lenght);

	windowed_dft(signal, out, windowed_signal, mod_array, arg_array, CHEBYSHEV, 1, signal_lenght); //will save both amplitude and phase

	for(i=0; i<signal_lenght; i++){
		if (i < half){
			//outputs every twiggle factor
			fprintf(ffft, "(%f,%f) ", creal(out[i]), cimag(out[i]));
			//outputs the amplitude 
			fprintf(fmod, "%f ",mod_array[i]);
			//outputs phase or principal argument
			fprintf(farg, "%f ",arg_array[i]);
		}
		//output the windowed signal 
		fprintf(fsignal, "%f ",windowed_signal[i]);
	}

	free(signal);
	free(mod_array);
	free(arg_array);
	free(out);
	free(windowed_signal);
	fclose(fsignal);
	fclose(fmod);
	fclose(farg);
	fclose(ffft);

	return 0;
}
