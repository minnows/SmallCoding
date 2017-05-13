//---------------------------------------------------------------------------

#ifndef TLRedH
#define TLRedH
#include <iostream>
#include <stdio.h>
#include "TLState.h"
#include "TLNetTraffic.h"
//---------------------------------------------------------------------------

class TLRed: public TLState
{
	private:
		TLNetTraffic* _context;

	public:
		TLRed(TLNetTraffic* context);
		void Handle();
};
#endif
