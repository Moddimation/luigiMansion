#include "Portable/msg.h"
#include "Os/dolphin/dolphin_trk_glue.h"
#include "trk.h"

DSError TRKMessageSend(TRK_Msg* msg)
{
	DSError write_err = TRKWriteUARTN(&msg->m_msg, msg->m_msgLength);
	return DS_NoError;
}
