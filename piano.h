

// STK
#include <Instrmnt.h>
#include <OnePole.h>
#include <OneZero.h>

using namespace stk;

#include "piano_coupled_strings.h"
#include "piano_soundboard.h"

class Piano : public Instrmnt
{
 public:
  Piano();
  virtual void noteOn (StkFloat frequency, StkFloat amplitude);
  virtual void noteOn (int keynum, StkFloat amplitude);
  virtual void noteOff (StkFloat amplitude);
  virtual void controlChange (int number, StkFloat value);
  virtual void setFrequency (StkFloat frequency);
  virtual StkFloat computeSample();

  // These are all ratios between [0, 1]
  void setEQBandwidthFactor(StkFloat factor);
  void setStiffnessFactor(StkFloat factor);
  void setDetuningFactor(StkFloat factor);
  void setBrightnessFactor(StkFloat factor);

  int getNoteNumber() { return noteNumber; }
  bool isActive();

  virtual StkFrames& tick( StkFrames& frames, unsigned int channel = 0 );
  virtual StkFloat tick( unsigned int channel = 0 );

 protected:
  void calcHammer(StkFloat velocity);
  void calcHighTuning(int noteNumber, StkFloat freq);

  Soundboard sb;
  CoupledStrings cs;
  OnePole hammerfilt[4];
  PoleZero dcBlock1;
  OneZero dcBlock2a;
  OnePole dcBlock2b;
  BiQuad eq;
  OneZero hipass;

  // High notes
  BiQuad high_bq[4];

  int noteNumber;
  StkFloat eqBandwidthFactor;
  StkFloat previousHammerPole;
  StkFloat brightnessFactor;

  int sample_counter;
  int zero_count;
};
