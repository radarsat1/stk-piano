
#include "piano_soundboard.h"
#include "piano_coefficients.h"

// Constants
#define PEDAL_ENVELOPE_T60 7.0
#define PEDAL_ENVELOPE_CUTOFF_TIME (1.4)
#define DRY_TAP_CUTOFF_TIME (.3)
#define DRY_PEDAL_RESONANCE_FACTOR (.5)
#define FIRST_DAMPERLESS_KEYNUM 89
#define MINUS_THREE_DB (0.70794578438413791);

Soundboard::Soundboard()
{
  sample_counter = 0;
  pedalPresenceFactor = 0;
}

void Soundboard::setPedalPresenceFactor(StkFloat factor)
{
  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;
  pedalPresenceFactor = factor; // range 0 - 1
}

StkFloat Soundboard::computeSample()
{
  StkFloat dt = dryTapAmp.tick();
  StkFloat pe = pedalEnv.tick();
  StkFloat nz = 0;

  if (dt > 0 || pe > 0)
    nz = noise.tick()*MINUS_THREE_DB;

  sample_counter ++;

  return dt*nz + pe*nz;
}

StkFrames& Soundboard::tick( StkFrames& frames, unsigned int channel )
{
#if defined(_STK_DEBUG_)
  if ( channel >= frames.channels() ) {
    errorString_ << "Delay::tick(): channel and StkFrames arguments are incompatible!";
    handleError( StkError::FUNCTION_ARGUMENT );
  }
#endif

  StkFloat *samples = &frames[channel];
  unsigned int hop = frames.channels();
  for ( unsigned int i=0; i<frames.frames(); i++, samples += hop ) {
    *samples = computeSample();
  }

  lastFrame_[0] = *(samples-hop);
  return frames;
}

StkFloat Soundboard::tick( unsigned int channel )
{
  lastFrame_[0] = computeSample();
  return lastFrame_[0];
}

void Soundboard::noteOn(int note, StkFloat velocity)
{
  sample_counter = 0;
  noteNumber = note;

  dryTapAmp.setT60( DryTapAmpT60.getValue(noteNumber) );
  dryTapAmp.setValue( DryTapAmpCurrent );
  dryTapAmp.keyOff();

  pedalEnv.setT60( PEDAL_ENVELOPE_T60 );
  float pedalEnvelopeCurrentValue = sustainPedalLevel.getValue(noteNumber) * pedalPresenceFactor;
  pedalEnv.setValue( DRY_PEDAL_RESONANCE_FACTOR * pedalEnvelopeCurrentValue );
  pedalEnv.keyOff();
}

void Soundboard::noteOff()
{
  if(noteNumber < FIRST_DAMPERLESS_KEYNUM){
    pedalEnv.setT60( PEDAL_ENVELOPE_CUTOFF_TIME );
    dryTapAmp.setT60( DRY_TAP_CUTOFF_TIME );
  }
}


