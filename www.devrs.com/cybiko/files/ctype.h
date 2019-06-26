/****************************************
/*  ctype.h								*
/*  Dustin Preuitt 2001					*
/*	version 1.0							*
/* An attempt to bring Cybiko C closer  *
/* ANSI C (after all, isn't that why we *
/* have standards?)						*
/***************************************/

#ifndef __CY_CTYPE_H__
#define __CY_CTYPE_H__

/* Returns nonzero if c is a digit or letter, zero otherwise.*/
int isalnum(int c);   /*  Tested.  */

/* Returns nonzero if c is a letter, zero otherwise.*/
int isalpha(int c);   /*  Tested.  */

/* Returns nonzero if c is a control character, zero otherwise.*/
int iscntrl(int c);   /*  Needs clarification on what is control character.  */

/* Returns nonzero if c is a digit, zero otherwise.*/
int isdigit(int c);	  /*  Tested.  */

/* Returns nonzero if c is a printing character (except space), zero otherwise.*/
int isgraph(int c);	  /*  Tested.  isgraph and isprint both need clarification on what prints  */

/* Returns nonzero if c is a printing character, zero otherwise.*/
int isprint(int c);   /*  See isgraph  */

/* Returns nonzero if c is a lowercase letter, zero otherwise.*/
int islower(int c);	  /*  Tested.  */

/* Returns nonzero if c is a punctuation character, zero otherwise.*/
int ispunct(int c);	  /*  Needs clarification on what is punctuation in upper ascii.  */

/* Returns nonzero if c is a space, zero otherwise.*/
int isspace(int c);   /*  Tested.  */

/* Returns nonzero if c is an uppercase letter, zero otherwise.*/
int isupper(int c);   /*  Tested.  */

/* Returns nonzero if c is a hex digit, zero otherwise.*/
int isxdigit(int c);  /*  Tested.  */


#endif