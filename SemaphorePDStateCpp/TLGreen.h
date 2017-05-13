//---------------------------------------------------------------------------

#ifndef TLGreenH
#define TLGreenH
#include <iostream>
#include <stdio.h>
#include "TLState.h"
#include "TLNetTraffic.h"
//---------------------------------------------------------------------------

class TLGreen: public TLState
{
	private:
		TLNetTraffic* _context;

	public:
		TLGreen(TLNetTraffic* context);
		void Handle();
};

#endif
