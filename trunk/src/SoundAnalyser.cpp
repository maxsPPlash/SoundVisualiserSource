#include "SoundAnalyser.h"

#include <fftw3.h>
#include <math.h>
#include <Windows.h>

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

void SoundAnalyser::Update() {
	fftwf_complex *in, *out;
	fftwf_plan p;

	const int num_items = 2048;
	const int bass_samples_cnt = 32;

	float *buf = snd->Data();
	data.fft_bass = 0;
	if(snd->DataSize() < num_items) {
		return;
	}

	/*Do fft to wav data*/
	in = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
	out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
	for (int i = 0; i < num_items; i++) {
		int id = i * 2;
		in[i][0] = buf[id];
		in[i][1] = 0;
	}

	p = fftwf_plan_dft_1d(num_items, in, out, FFTW_FORWARD, FFTW_MEASURE);  // FFTW_ESTIMATE    //1D Complex DFT, fftwf_FORWARD & BACKWARD just give direction and have particular values
	fftwf_execute(p);

	for (int i = 0; i < 512; ++i) {
		float r1 = sqrt (sqrt( pow(out[i][0],2) + pow(out[i][1],2)));
//		float r2 = sqrt (sqrt( pow(out[i*2+1][0],2) + pow(out[i*2+1][1], 2)));
		float val = min(sqrt(r1) / 3.f, 1.f);
		fft_data[i] = 255 * val;        //2 sqrt since np.sqrt( np.abs() )
		if (i < bass_samples_cnt)
			data.fft_bass += val;
	}
	data.fft_bass /= bass_samples_cnt;
	data.fft_data = fft_data;

	fftwf_free(in); fftwf_free(out);
	fftwf_destroy_plan(p);
}

void SoundAnalyser::GetData(SoundData &res) {
	res = data;
}