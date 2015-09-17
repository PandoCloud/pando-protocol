#include "platform_functions.h"

void FUNCTION_ATTRIBUTE show_package(uint8_t *buffer, uint16_t length)
{
	int i = 0;
	pd_printf("Package length: %d\ncontent is: \n", length);

	for (i = 0; i < length; i++)
	{
		pd_printf("%02x ",(uint8_t)buffer[i]);
	}

	pd_printf("\n");
}

