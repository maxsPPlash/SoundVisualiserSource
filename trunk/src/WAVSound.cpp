#include "WAVSound.h"
#include<sndfile.h>

void WAVSound::Update(float cur_time) {
	int dast_semple = cur_time * samplerate * 2;
	if (dast_semple - cur_sample > 2048 * 2)
		cur_sample += 2048 * 2;
//	cur_sample = cur_time * samplerate * 2;
}

bool WAVSound::LoadSound(const char *file_path) {
	SNDFILE *file = NULL;
	path = file_path;

	SF_INFO sfinfo;
	int num_channels;
	int num, num_items;
//	double *buf;
	int ch;

	//Read the file, into buffer
	file = sf_open(file_path, SFM_READ, &sfinfo);

	/* Print some of the info, and figure out how much data to read. */
	sample_count = sfinfo.frames;
	samplerate = sfinfo.samplerate;
	ch = sfinfo.channels;
	printf("frames=%d\n", sample_count);
	printf("samplerate=%d\n", samplerate);
	printf("channels=%d\n", ch);
	num_items = sample_count * ch;
	printf("num_items=%d\n", num_items);

	//Allocate space for the data to be read, then read it
	buf = (float *)malloc(num_items * sizeof(float));
	num = sf_read_float(file, buf, num_items);

	sf_close(file);
	printf("Read %d items\n", num);

	return 0;
}