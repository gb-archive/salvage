/****************************************
/*  assert.h							*
/*  Dustin Preuitt 2001					*
/*	v 1.0								*
/* An attempt to bring Cybiko C closer  *
/* ANSI C (after all, isn't that why we *
/* have standards?)						*
/***************************************/

#ifndef __CY_ASSERT_H__
#define __CY_ASSERT_H__

#include "CyWin.h"

#define assert(n)  if(!(n)){cprintf("Assertion failed: %s  in File %s Line %d\n","n", __FILE__,__LINE__ -1);exit(0);}






#endif