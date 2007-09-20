
#include "AsympT60.h"
#include <math.h>

/* T48COEFF times the "time constant" gives time to decay 48dB exponentially */
#define T48COEFF 5.52620422318571
/* JOS - I get 45.6 dB decay from the above constant, using the formula
   -20 Log[10,Exp[-5.25]] in Mathematica.  The formula for T48COEFF is
   -N[Log[10^(-48/20)]] = 5.52620422318571
*/


void AsympT60::setT60(StkFloat t60)
{
	 factor_ = exp(-7.0/(t60*Stk::sampleRate()));
	 constant_ = (1.0 - factor_) * target_;
}

void AsympT60::setT48(StkFloat t48)
{
	 factor_ = exp(-T48COEFF/(t48*Stk::sampleRate()));
	 constant_ = (1.0 - factor_) * target_;
}
