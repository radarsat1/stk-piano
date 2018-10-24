
#ifndef _ASYMP_H_
#define _ASYMP_H_

#include <Asymp.h>

using namespace stk;

class AsympT60 : public Asymp
{
  public:
    AsympT60() : Asymp() {}
	
	void setT60(StkFloat t60);
	void setT48(StkFloat t48);
};

#endif // _ASYMP_H_
