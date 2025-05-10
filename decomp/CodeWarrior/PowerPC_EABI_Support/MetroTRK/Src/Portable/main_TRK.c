#include "Portable/main_TRK.h"
#include "Portable/nubinit.h"

static DSError TRK_mainError;

DSError TRK_main(void)
{
	TRK_mainError = TRKInitializeNub();

	if (TRK_mainError == DS_NoError) {
		TRKNubWelcome();
		TRKNubMainLoop();
	}

	TRK_mainError = TRKTerminateNub();
	return TRK_mainError;
}
