
#include "piano_soundboard.h"
#include "piano_coefficients.h"

// Constants
#define PEDAL_ENVELOPE_T60 7.0
#define PEDAL_ENVELOPE_CUTOFF_TIME (1.4)

Soundboard::Soundboard()
{
	 sample_counter = 0;
}

StkFloat Soundboard::computeSample()
{
	 StkFloat dt = dryTapAmp.tick();
	 StkFloat pe = pedalEnv.tick();
	 StkFloat nz = 0;

	 if (dt > 0 || pe > 0)
		  nz = noise.tick();

	 sample_counter ++;

	 return (dt*nz + pe*nz)*0.5;
}

void Soundboard::noteOn(int note, StkFloat velocity)
{
	 sample_counter = 0;
	 noteNumber = note;

	 dryTapAmp.setT60( DryTapAmpT60.getValue(noteNumber) * velocity );
	 dryTapAmp.setValue( DryTapAmpCurrent );
	 dryTapAmp.keyOff();

	 pedalEnv.setValue( sustainPedalLevel.getValue(noteNumber) );
	 pedalEnv.setTarget( sustainPedalLevel.getValue(noteNumber) );
}

void Soundboard::noteOff()
{
	 pedalEnv.setT60(PEDAL_ENVELOPE_CUTOFF_TIME);
	 pedalEnv.keyOff();
}


