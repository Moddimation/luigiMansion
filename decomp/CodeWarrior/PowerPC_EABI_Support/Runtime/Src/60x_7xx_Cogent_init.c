//////////////////////////////////////////////////////////////////////////////
//
// Cogent PPC board-specific initialization code. 
//
//
//////////////////////////////////////////////////////////////////////////////

#pragma section all_types ".init" ".init"	

//////////////////////////////////////////////////////////////////////
//
//	init_board
//
//	Performs board-specific initializations.
//
//	For the Cogent PPC board, none are required, so this is just a
//	stub.
//
//////////////////////////////////////////////////////////////////////

asm void init_board()
{
	nofralloc
	
	blr
}