#include "WAVSoundFile.h"
#include<sndfile.h>

bool WAVSoundFile::LoadSound(const char *file_path) {
	SNDFILE *file = NULL;
	path = file_path;

	SF_INFO sfinfo;
	int num_channels;
	int num, num_items;
//	double *buf;

	//Read the file, into buffer
	file = sf_open(file_path, SFM_READ, &sfinfo);

	/* Print some of the info, and figure out how much data to read. */
	sample_count = sfinfo.frames;
	samplerate = sfinfo.samplerate;
	channels = sfinfo.channels;
//	printf("frames=%d\n", sample_count);
//	printf("samplerate=%d\n", samplerate);
//	printf("channels=%d\n", ch);
	num_items = sample_count * channels;
//	printf("num_items=%d\n", num_items);

	//Allocate space for the data to be read, then read it
	data = (float *)malloc(num_items * sizeof(float));
	num = sf_read_float(file, data, num_items);

	sf_close(file);
//	printf("Read %d items\n", num);

	return 0;
}