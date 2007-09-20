
#include <math.h>
#include "piano_coupled_strings.h"
#include "piano_coefficients.h"

#define LOOP_GAIN_CUTOFF_T60 (.05)
#define MAX_LOOP_GAIN (.9996)
#define FIRST_DAMPERLESS_KEYNUM 89

CoupledStrings::CoupledStrings()
	 : Effect(),
	   delay1(300, 2048), delay2(301, 2048)
{
	 // initialize control parameters
	 detuningFactor = 1;
	 stiffnessFactor = 0.3;
	 setFrequency(440);

	 loopGain.setValue( MAX_LOOP_GAIN );
	 loopGain.setTarget( MAX_LOOP_GAIN );

	 // feedback values
	 prev_y1 = prev_y2 = 0;
}

StkFloat CoupledStrings::AllPassPhase(StkFloat a1, StkFloat wT)
{
	 // Return the phase delay of a pole-zero allpass filter
	 return atan2((a1*a1-1.0)*sin(wT),(2.0*a1+(a1*a1+1.0)*cos(wT)));
}

StkFloat CoupledStrings::PoleZeroPhase(StkFloat b0, StkFloat b1, StkFloat a1, StkFloat wT)
{
	 // Return the phase delay of a pole-zero filter
	 StkFloat temp = atan2(-b1*sin(wT)*(1 + a1*cos(wT)) + a1*sin(wT)*(b0 + b1*cos(wT)),
						   (b0 + b1*cos(wT))*(1 + a1*cos(wT)) + b1*sin(wT)*a1*sin(wT));
	 return temp;
}

StkFloat CoupledStrings::delayLength(StkFloat freq, StkFloat stiffnessCoefficient)
{
	 // Calculate delay length for a particular frequency
	 // accounting for stiffness allpass filter delays
	 // and coupling filter delay
	 
	 StkFloat wT = freq * 2*M_PI / Stk::sampleRate();
	 
	 int num_strings = 2;
	 StkFloat len = (2*M_PI
					 + 3*AllPassPhase(stiffnessCoefficient, wT)
					 + PoleZeroPhase(1 + num_strings*(couplingB0),
									 couplingA1 + num_strings*couplingB1,
									 couplingA1, wT)) 
		             /wT;

	 return len;
}

int CoupledStrings::FrequencyToNoteNumber(StkFloat frequency)
{
	 return (int)((log(frequency)-log(440.0))/log(2.0)*12+69+0.5);
}

StkFloat CoupledStrings::NoteNumberToFrequency(int note)
{
	 return (StkFloat) (440.0/32.0) * pow( 2, (note - 9) / 12.0 );
}

void CoupledStrings::setDetuningFactor(StkFloat factor)
{
	 if (factor < 0) factor = 0;
	 if (factor > 1) factor = 1;
	 detuningFactor = factor*10; // range 0 - 10

	 setFrequency(frequency);
}

void CoupledStrings::setStiffnessFactor(StkFloat factor)
{
	 if (factor < 0) factor = 0;
	 if (factor > 1) factor = 1;

	 stiffnessFactor = factor * 2.7 + 1;  // range 1 - 3.7

	 setFrequency(frequency);
}

void CoupledStrings::calcCouplingFilter()
{
	// Coupling Filter
	StkFloat attenuationPerPeriod = pow(10.0,((singleStringDecayRate.getValue(noteNumber)/frequency)/20.0));
	StkFloat g = attenuationPerPeriod;
	StkFloat b = singleStringZero.getValue(noteNumber);
	StkFloat a = singleStringPole.getValue(noteNumber);

	StkFloat tempd = 3*(1-b)-g*(1-a);

	couplingB0 = 2*(g*(1-a)-(1-b))/tempd;
	couplingB1 = 2*(a*(1-b)-g*(1-a)*b)/tempd;
	couplingA1 = (g*(1-a)*b - 3*a*(1-b))/tempd;

	couplingPoleZero.setB0(couplingB0);
	couplingPoleZero.setB1(couplingB1);
	couplingPoleZero.setA1(couplingA1);
}

void CoupledStrings::setFrequency(StkFloat freq)
{
	frequency = freq;
	noteNumber = FrequencyToNoteNumber(frequency);

	StkFloat hz = detuningHz.getValue(noteNumber);

	freq1 = frequency + 0.5*hz*detuningFactor;
	freq2 = frequency - 0.5*hz*detuningFactor;

	// Coupling Filter
	calcCouplingFilter();

	// Stiffness Allpasses
	StkFloat stiffness = stiffnessFactor * stiffnessCoefficient.getValue(noteNumber);

	// Stability protection needed for lower notes (SS)
	if (stiffness < -0.999) stiffness = -0.95;

	for (int i=0; i<6; i++)
		stiffnessAP[i].setAllpass(stiffness);

	delay1.setDelay(delayLength(freq1, stiffness));
	delay2.setDelay(delayLength(freq2, stiffness));
}

void CoupledStrings::noteOn (StkFloat frequency, StkFloat amplitude)
{
	setFrequency(frequency);

	loopGain.setValue( MAX_LOOP_GAIN );
	loopGain.setTarget( MAX_LOOP_GAIN );
}

void CoupledStrings::noteOff (StkFloat amplitude)
{
	if (noteNumber < FIRST_DAMPERLESS_KEYNUM) {
		 loopGain.setTarget( releaseLoopGain.getValue(noteNumber) );
		 loopGain.setT60( LOOP_GAIN_CUTOFF_T60 );
	}
}

void CoupledStrings::controlChange (int number, StkFloat value)
{
}

StkFloat CoupledStrings::computeSample(StkFloat input)
{
	 int i;
	 StkFloat y, y1, y2;
	 y1 = y2 = input * loopGain.tick();

	 for (i=0; i<3; i++) {
		  y1 = stiffnessAP[i].tick(y1 + prev_y1);
		  y2 = stiffnessAP[i].tick(y2 + prev_y2);
		  prev_y1 = 0;
		  prev_y2 = 0;
	 }

	 y1 = delay1.tick(y1);
	 y2 = delay2.tick(y2);

	 y = couplingPoleZero.tick(y1 + y2);
	 prev_y1 = y1 + y;
	 prev_y2 = y2 + y;

	 return y1 + y2;
}

