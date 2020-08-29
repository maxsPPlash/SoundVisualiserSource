#include "SoundAnalyser.h"

#define _USE_MATH_DEFINES
#include <fftw3.h>
#include <math.h>
#include <Windows.h>
#include "Utils.h"

/* https://www.youtube.com/watch?v=GDKFSAXZwtc
Natural
f(x)=x*Fit_factor

Exponential
f(x)=log(x*Fit_factor2)*Fit_factor

Multi Peak Scale
f(x,i)=x/Peak[i]*Fit_factor

Max Peak Scale
f(x)=x/Global_Peak*Fit_factor
*/

bool SoundAnalyser::CalcFFT_log(float *input, int input_sz, int input_stride, float *output, int output_sz) {
	fftwf_complex *in, *out;
	fftwf_plan p;

	in = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * input_sz);
	out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * input_sz);
	for (int i = 0; i < input_sz; i++) {
		// Hann function
//		float multiplier =1.f;// 0.5 * (1 - cos(2*M_PI*i/2047));
		float multiplier = 0.54 - 0.46 * cos(2*M_PI*i/2047);
		int id = i * input_stride;
		in[i][0] = multiplier*input[id];
		in[i][1] = 0;
	}

	p = fftwf_plan_dft_1d(input_sz, in, out, FFTW_FORWARD, FFTW_MEASURE);  // FFTW_ESTIMATE    //1D Complex DFT, fftwf_FORWARD & BACKWARD just give direction and have particular values
	fftwf_execute(p);

	for (int i = 0; i < output_sz; ++i) {
		float r1 = (sqrt(pow(out[i][0],2) + pow(out[i][1],2)));
		output[i] = r1 * 0.02f;
//		output[i] = (log10(10.f + r1*100.f) - 1.f) / 3.f;
	}

	fftwf_free(in); fftwf_free(out);
	fftwf_destroy_plan(p);

	return true;
}

//void SoundAnalyser::Update() {
//	fftwf_complex *in, *out;
//	fftwf_plan p;
//
//	const int num_items = 2048;
//	const int bass_samples_cnt = 32;
//
//	float *buf = snd->Data();
//	data.fft_bass = 0;
//	if(snd->DataSize() < num_items) {
//		return;
//	}
//
//	/*Do fft to wav data*/
//	in = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
//	out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
//	for (int i = 0; i < num_items; i++) {
//		// Hann function
//		double multiplier =1.f;// 0.5 * (1 - cos(2*M_PI*i/2047));
//		int id = i * 2;
//		in[i][0] = multiplier*(buf[id]+buf[id+1])/2.f;
//		in[i][1] = 0;
//	}
//
//	p = fftwf_plan_dft_1d(num_items, in, out, FFTW_FORWARD, FFTW_MEASURE);  // FFTW_ESTIMATE    //1D Complex DFT, fftwf_FORWARD & BACKWARD just give direction and have particular values
//	fftwf_execute(p);
//
//	for (int i = 0; i < 512; ++i) {
//		float r1 = (sqrt( pow(out[i][0],2) + pow(out[i][1],2)));
////		float dbValue = log10(r1);
////            magnitude = Math.round(dbValue * 8);
////		float r2 = sqrt (sqrt( pow(out[i*2+1][0],2) + pow(out[i*2+1][1], 2)));
//		float val = (log10(10.f + r1*100.f) - 1.f) / 3.f;// r1 / 10.f;// min(sqrt(r1) / 3.f, 1.f);
//		unsigned char new_data = 255 * clamp(val, 0.f, 1.f);
//		if (fft_data[i] < new_data)
//			fft_data[i] = new_data;
//		else
//			fft_data[i] -= min(fft_data[i] - new_data, 3);
////		fft_data[i] = 255 * clamp(val, 0.f, 1.f);        //2 sqrt since np.sqrt( np.abs() )
//		if (i < bass_samples_cnt)
//			data.fft_bass += val;
//	}
//	data.fft_bass /= bass_samples_cnt;
//	data.fft_data = fft_data;
//
//	fftwf_free(in); fftwf_free(out);
//	fftwf_destroy_plan(p);
//}
//
//void SoundAnalyser::GetData(SoundData &res) {
//	res = data;
//}