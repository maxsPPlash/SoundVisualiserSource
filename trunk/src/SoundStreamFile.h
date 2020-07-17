#pragma once
#include "ISoundStream.h"
#include "ISoundFile.h"

class SoundStreamFile : public ISoundStream {
public:
	SoundStreamFile(ISoundFile *sound_file, int sample_step);
	virtual ~SoundStreamFile() {}

	virtual bool Update(float cur_time) override;
	virtual float *CurData() { return file->Data() + cur_sample * file->Channels(); }

	ISoundFile *File() { return file; }
	bool Finished() { return finished; }

private:
	ISoundFile *file;

	int cur_sample;
	int step;
	bool finished;
};