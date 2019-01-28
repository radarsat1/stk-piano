
#ifndef _PIANO_SOUNDBOARD_H_
#define _PIANO_SOUNDBOARD_H_

#include <Noise.h>
#include <Generator.h>
#include <Asymp.h>

using namespace stk;


class Soundboard : public Generator
{
 public:
  Soundboard();
  ~Soundboard() {}

  virtual StkFloat computeSample();
  void noteOn(int noteNumber, StkFloat velocity);
  void noteOff();

  void setPedalPresenceFactor(StkFloat factor);

  virtual StkFrames& tick( StkFrames& frames, unsigned int channel = 0 );
  virtual StkFloat tick( unsigned int channel = 0 );

 protected:
  int sample_counter;
  int noteNumber;
  StkFloat pedalPresenceFactor;

  Noise noise;
  Asymp dryTapAmp;
  Asymp pedalEnv;
};

#endif // _PIANO_SOUNDBOARD_H_
