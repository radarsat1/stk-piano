
#ifndef _PIANO_SOUNDBOARD_H_
#define _PIANO_SOUNDBOARD_H_

#include <Noise.h>
#include <Generator.h>
#include "AsympT60.h"

class Soundboard : public Generator
{
  public:
	Soundboard();
	~Soundboard() {}

	virtual StkFloat computeSample();
	void noteOn(int noteNumber, StkFloat velocity);
	void noteOff();

  protected:
	int sample_counter;
	int noteNumber;

	Noise noise;
	AsympT60 dryTapAmp;
	AsympT60 pedalEnv;
};

#endif // _PIANO_SOUNDBOARD_H_
