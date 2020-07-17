#include "SoundStreamFile.h"

SoundStreamFile::SoundStreamFile(ISoundFile *sound_file, int sample_step) : file(sound_file), step(sample_step) {
	finished = !file || file->SampleCount() < step;
}

bool SoundStreamFile::Update(float cur_time) {
	if (finished) return false;

	const int ch = file->Channels();
	const int dest_sample = cur_time * file->SampleRate();
	if (dest_sample - cur_sample > step) {
		cur_sample += step;
		finished = file->SampleCount() < cur_sample + step;
		return true;
	}
	return false;

	// mb as other mode?
//	cur_sample = cur_time * file->SampleRate() * ch;
//	return true;
}