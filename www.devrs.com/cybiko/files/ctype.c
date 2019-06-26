/****************************************
/*  ctype.c								*
/*  Dustin Preuitt 2001					*
/*	version 1.0							*
/* An attempt to bring Cybiko C closer  *
/* ANSI C (after all, isn't that why we *
/* have standards?)						*
/***************************************/


#include "ctype.h"


int isalnum(int c)
{
	if ((isalpha(c)) || (isdigit(c)))
		return 1;
	return 0;
}

int isalpha(int c)
{
	if ((isupper(c)) || (islower(c)))
	  return 1;
	return 0;
}

/* Not sure which characters are actually control characters in Cybikoland */
int iscntrl(int c)
{
	if ((c >= 0) && (c <= 31))
		return 1;
	return 0;
}

int isdigit(int c)
{
	if ((c >= 48) && (c <= 57))
		return 1;
	return 0;
}

/*  Not quite sure this one is correct either */
int isgraph(int c)
{
	if ((c >= 33) && (c <= 255))
	  return 1;
	return 0;
}


int islower(int c)
{
	if ((c >= 97) && (c <= 122))
		return 1;
	return 0;
}

/*  Not quite sure this one is correct either */
int isprint(int c)
{
	if ((c >= 32) && (c <= 255))
	  return 1;
	return 0;
}


int ispunct(int c)
{
	if ((!isalnum(c)) && (!isspace(c)) && (isprint(c)))
		return 1;
	return 0;
}

int isspace(int c)
{
	if (c == 32)
		return 1;
	return 0;
}

int isupper(int c)
{
	if ((c >= 65) && (c <= 90))
		return 1;
	return 0;
}

/* Returns a nonzero value if c is a hex digit (0-9,a-f,A-F), zero otherwise */
int isxdigit(int c)
{
	if (((c >= 48) && (c<=57)) || ((c >= 65) && (c <= 70)) || ((c >= 97) && (c <= 102)))
		return 1;
	return 0;
}
