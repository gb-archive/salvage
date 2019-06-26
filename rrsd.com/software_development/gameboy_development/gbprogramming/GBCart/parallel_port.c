#include "parallel_port.h"

/* init port */
void init_port(int port)
{
	data_port = *(unsigned far *)MK_FP(0x0040, 6 + 2*port);
	if(data_port == 0)
	{
		printf("Can't find address of parallel port %d...\n", port);
		exit(1);
	} else {
		status_port = data_port + 1;
		ctrl_port   = data_port + 2;
		if(data_port != 0x378) printf("Parallel port %d is located at %X-%X\n", port, data_port, ctrl_port);
	}
}


