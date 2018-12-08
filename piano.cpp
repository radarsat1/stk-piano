
#include <ctime>
#include <math.h>

// Piano model
#include "piano.h"
#include "piano_coefficients.h"
#include "piano_coupled_strings.h"
#include "piano_soundboard.h"

// Constants
#define DCB2_TURNOFF_KEYNUM 92
#define LOWEST_STABLE_NOTE 1  // was fixed by limiting stiffness poles
#define HIGHEST_NOTEOFF_NOTE 86
#define FIRST_HIGH_NOTE 100

#define dbinv(x) pow(10,.05*x)

Piano::Piano()
  : Instrmnt()
{
  sample_counter = 0;
  zero_count = 0;
  hipass.setB0(0.5);
  hipass.setB0(-0.5);
  noteNumber = 0;
  overallGain = 0;
  brightnessFactor = 0;
}

void Piano::noteOn(StkFloat freq, StkFloat amp)
{
  noteOn( cs.FrequencyToNoteNumber(freq), amp);
}

void Piano::noteOn(int noteNumber, StkFloat amp)
{
  StkFloat freq = cs.NoteNumberToFrequency(noteNumber);

  if (noteNumber < LOWEST_STABLE_NOTE)
    return;

  setFrequency( freq );

  sb.noteOn(noteNumber, amp);

  if (noteNumber < FIRST_HIGH_NOTE)
    cs.noteOn(freq, amp);

  calcHammer(amp);

  // Count time in samples since noteOn
  sample_counter = 0;

  // DC Blocking filter
  StkFloat dcb_a1 = DCBa1.getValue(noteNumber);
  StkFloat dcb_b0 = 1.0 - dcb_a1;

  dcBlock1.setA1(dcb_a1);
  dcBlock1.setB0(dcb_b0 *  0.5);
  dcBlock1.setB1(dcb_b0 * -0.5);

  // Second DC-Blocking filter for high notes only
  if (noteNumber >= DCB2_TURNOFF_KEYNUM) {
    dcBlock2a.setB0(0.5);
    dcBlock2a.setB1(-0.5);
    dcBlock2b.setB0(dcb_b0);
    dcBlock2b.setA1(dcb_a1);
  }
  else {
    dcBlock2a.setB0(1);
    dcBlock2a.setB1(0);
    dcBlock2b.setB0(1);
    dcBlock2b.setA1(0);
  }

  // High note biquads
  if (noteNumber >= FIRST_HIGH_NOTE) {
    calcHighTuning(noteNumber, freq);
  }

  // Strike position comb filter EQ
  StkFloat eq_tuning = freq / strikePosition.getValue(noteNumber);
  StkFloat eq_bandwidth = EQBandwidthFactor.getValue(noteNumber) * freq;

  eq.setResonance(eq_tuning, eq_bandwidth / Stk::sampleRate(), true);
  eq.setGain( EQGain.getValue(noteNumber) );
}

void Piano::calcHighTuning(int noteNumber, StkFloat freq)
{
  //Highnote tuning

  StkFloat r1_1 = dbinv( r1_1db.getValue(noteNumber) / Stk::sampleRate() );
  StkFloat r1_2 = dbinv( r1_2db.getValue(noteNumber) / Stk::sampleRate() );
  StkFloat r2   = dbinv( r2db.getValue(noteNumber)   / Stk::sampleRate() );
  StkFloat r3   = dbinv( r3db.getValue(noteNumber)   / Stk::sampleRate() );
  StkFloat e    = dbinv( secondStageAmpRatio.getValue(noteNumber) );
  StkFloat second_partial_factor = secondPartialFactor.getValue(noteNumber);
  StkFloat third_partial_factor = thirdPartialFactor.getValue(noteNumber);

  // Decay
  // Note: The original patch calles for bq4_gEarBalled to be assigned to
  //       the decay of the first BiQuad.  However, my ears tell me that
  //       it works much better to spread the loss over 2 of the biquads.
  //       Otherwise it is very obvious when we switch models because of
  //       an audible change in loudness. (SS)

  high_bq[0].setGain( bq4_gEarBalled.getValue(noteNumber) / 0.5 );
  high_bq[1].setGain( bq4_gEarBalled.getValue(noteNumber) / 0.5 );

  // Note: patch has bq numbering out of order:
  //       here we map bq4 = [0]
  //                   bq3 = [1]
  //                   bq1 = [2]
  //                   bq2 = [3]

  StkFloat costheta3 = cos(TWO_PI*freq*third_partial_factor / Stk::sampleRate());
  high_bq[0].setB1( 0 );
  high_bq[0].setB2( 0 );
  high_bq[0].setA1( -2.0*r3*costheta3 );
  high_bq[0].setA2( r3*r3 );

  StkFloat costheta2 = cos(TWO_PI*freq*second_partial_factor / Stk::sampleRate());
  high_bq[1].setB1( 0 );
  high_bq[1].setB2( 0 );
  high_bq[1].setA1( -2.0*r2*costheta2 );
  high_bq[1].setA2( r2*r2 );

  StkFloat costheta1 = cos(TWO_PI*freq/Stk::sampleRate());
  high_bq[2].setB1( -2.0*(e*r1_1+(1-e)*r1_2)*costheta1 );
  high_bq[2].setB2( e*r1_1*r1_1+(1-e)*r1_2*r1_2 );
  high_bq[2].setA1( -2.0*r1_1*costheta1 );
  high_bq[2].setA2( r1_1*r1_1 );

  high_bq[3].setB1( 0 );
  high_bq[3].setB2( 0 );
  high_bq[3].setA1( -2.0*r1_2*costheta1 );
  high_bq[3].setA2( r1_2*r1_2 );
}

void Piano::noteOff (StkFloat amplitude)
{
  sb.noteOff();
  cs.noteOff(0);
}

void Piano::controlChange (int number, StkFloat value)
{
}

void Piano::setFrequency (StkFloat frequency)
{
  noteNumber = CoupledStrings::FrequencyToNoteNumber(frequency);

  if (noteNumber < FIRST_HIGH_NOTE)
    cs.setFrequency(frequency);
}

void Piano::setStiffnessFactor(StkFloat factor)
{
  cs.setStiffnessFactor(factor);
}

void Piano::setDetuningFactor(StkFloat factor)
{
  cs.setDetuningFactor(factor);
}

void Piano::setOverallGain(StkFloat factor)
{
  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;
  overallGain = factor * 3; // range 0 - 3
}

void Piano::setPedalPresenceFactor(StkFloat factor)
{
  sb.setPedalPresenceFactor(factor);
}

void Piano::setBrightnessFactor(StkFloat factor)
{
  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;
  brightnessFactor = factor * 0.25 + -0.25; // range -0.25 - 0
}

void Piano::calcHammer(StkFloat velocity)
{
  // Hammer filter calculations

  StkFloat loudPoleValue = loudPole.getValue(noteNumber);
  loudPoleValue += brightnessFactor;
  StkFloat softPoleValue = softPole.getValue(noteNumber);
  // NOT APPLIED. Unchecked in the inspector
//  StkFloat normalizedVelocityValue = normalizedVelocity.getValue(velocity);
  StkFloat normalizedVelocityValue = velocity;
  StkFloat loudGainValue = loudGain.getValue(noteNumber);
  StkFloat softGainValue = softGain.getValue(noteNumber);

  StkFloat hammerPole = softPoleValue + (loudPoleValue - softPoleValue)*normalizedVelocityValue;
  StkFloat hammerGain = overallGain*(softGainValue + (loudGainValue - softGainValue)*normalizedVelocityValue);

  StkFloat HammerFiltA1 = -hammerPole;
  StkFloat HammerFiltB0 = (1.0-hammerPole)*hammerGain;

  // Unused for now
  /*
   StkFloat loudHammerFiltA1 = -loudPoleValue;
   StkFloat loudHammerFiltB0 = (1.0-loudPoleValue)*hammerGain;
   */

  int i;
  for (i=0; i<4; i++) {
    hammerfilt[i].setB0(HammerFiltB0);
    hammerfilt[i].setA1(HammerFiltA1);
  }
}

StkFloat Piano::computeSample()
{
  StkFloat f;
  int i;

  // Send dummy noteOffs above E6
  if ( noteNumber > 86
      && (sample_counter / (StkFloat)Stk::sampleRate()
          > noteOffDelayTime.getValue(noteNumber)) )
  {
    sb.noteOff();
    cs.noteOff(0);
  }

  // Track time since noteOn
  // (for dummy noteOff calculations)
  sample_counter ++;

  // Soundboard
  f = sb.tick();

  if (noteNumber >= FIRST_HIGH_NOTE)
  {
    // High notes
    f = hipass.tick(f);
    f = dcBlock1.tick(f);
    for (i=0; i<4; i++) {
      f = hammerfilt[i].tick(f);
      f = high_bq[i].tick(f);
    }
  }
  else
  {
    // Linearized Hammer
    for (i=0; i<4; i++)
      f = hammerfilt[i].tick(f);

    // DC-Blocking 1
    f = dcBlock1.tick(f);

    // Coupled strings
    f = cs.tick(f);

    // DC-Blocking 2a
    f = dcBlock2a.tick(f);

    // Strike position comb filter EQ
    f = eq.tick(f) + f;

    // DC-Blocking 2b
    f = dcBlock2b.tick(f);
  }

  // Threshold amplitude for activity detection
  // (From observation)
  if (fabs(f) < 0.0002) {
    if (zero_count < 10000)
      zero_count++;
  }
  else
    zero_count=0;

  return f;
}

bool Piano::isActive()
{
  return zero_count < 1000;
}

StkFrames& Piano::tick( StkFrames& frames, unsigned int channel )
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

StkFloat Piano::tick( unsigned int channel )
{
  lastFrame_[0] = computeSample();
  return lastFrame_[0];
}
