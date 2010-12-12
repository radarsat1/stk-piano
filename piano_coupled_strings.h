
#ifndef _PIANO_COUPLED_STRINGS_H_
#define _PIANO_COUPLED_STRINGS_H_

#include "Generator.h" // STK
#include "BiQuad.h"
#include "DelayA.h"
#include "PoleZero.h"
#include "TwoZero.h"
#include "Effect.h"

using namespace stk;

#include "AsympT60.h"

class CoupledStrings : public Effect
{
  public:
	CoupledStrings();
	~CoupledStrings() {}

	virtual void clear() {};

	virtual void noteOn (StkFloat frequency, StkFloat amplitude); // needed?
	virtual void noteOff (StkFloat amplitude); // needed?
	virtual void controlChange (int number, StkFloat value); // needed?
	virtual void setFrequency (StkFloat frequency);

	// Parameters
	void setDetuningFactor(StkFloat factor);
	void setStiffnessFactor(StkFloat factor);

	// Helpers
	static StkFloat AllPassPhase(StkFloat a1, StkFloat wT);
	static StkFloat PoleZeroPhase(StkFloat b0, StkFloat b1, StkFloat a1, StkFloat wT);	
	static int FrequencyToNoteNumber(StkFloat frequency);
	static StkFloat NoteNumberToFrequency(int note);

    virtual StkFrames& tick( StkFrames& frames, unsigned int channel = 0 );
    virtual StkFloat tick( StkFloat, unsigned int channel = 0 );

  protected:
	StkFloat delayLength(StkFloat freq, StkFloat stiffnessCoefficient);
	void calcCouplingFilter();

	StkFloat computeSample(StkFloat input);
	
	PoleZero stiffnessAP[6];
	DelayA delay1, delay2;
	
	PoleZero couplingPoleZero;

	AsympT60 loopGain;

	StkFloat prev_y1, prev_y2;

	StkFloat detuningFactor;
	StkFloat frequency, freq1, freq2;
	StkFloat stiffnessFactor;
	int noteNumber;

	StkFloat couplingB0;
	StkFloat couplingB1;
	StkFloat couplingA1;
};

#endif // _PIANO_COUPLED_STRINGS_H_
