//---------------------------------------------------------------------------

#ifndef TLNetTrafficH
#define TLNetTrafficH
#include "TLState.h"
//---------------------------------------------------------------------------
class TLNetTraffic
{
	private:
		TLState* _state;
	public:
		TLNetTraffic();
		void setState ( TLState* state );
		void Handle();
};
#endif
