// InpoutTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
/* ----Prototypes of Inp and Outp--- */

short _stdcall Inp32(short PortAddress);
void _stdcall Out32(short PortAddress, short data);

/*--------------------------------*/

int main(int argc, char* argv[])
{

	int data;

	if(argc<3)
	{
		//too few command line arguments, show usage
		printf("Error : too few arguments\n\n***** Usage *****\n\nInpoutTest read <ADDRESS> \nor \nInpoutTest write <ADDRESS> <DATA>\n\n\n\n\n");
	} 
	else if(!strcmp(argv[1],"read"))
	{

		data = Inp32(atoi(argv[2]));

		printf("Data read from address %s is %d \n\n\n\n",argv[2],data);
	
	}
	else if(!strcmp(argv[1],"write"))
	{
		if(argc<4)
		{
			printf("Error in arguments supplied");
			printf("\n***** Usage *****\n\nInpoutTest read <ADDRESS> \nor \nInpoutTest write <ADDRESS> <DATA>\n\n\n\n\n");
		}
		else
		{
		Out32(atoi(argv[2]),atoi(argv[3]));
		printf("data written to %s\n\n\n",argv[2]);
		}
	}



	return 0;
}
