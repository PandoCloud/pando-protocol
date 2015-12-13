//  Copyright (c) 2015 Pando. All rights reserved.
//  PtotoBuf:   ProtocolBuffer.h
//
//  Create By ZhaoWenwu On 15/01/24.

#include "pando_endian.h"

int little_endian = -1;

int FUNCTION_ATTRIBUTE is_little_endian()
{
	if(little_endian == -1)
	{
		union int_char
		{
			int i;
			char c;
		}x;
		x.i = 1;
		little_endian = x.c;
	}

	return little_endian;
}

uint16_t FUNCTION_ATTRIBUTE net16_to_host(uint16_t A)
{
	if(is_little_endian())
	{
		return ((((A) & 0xff00) >> 8) | (((A) & 0x00ff) << 8));
	}
	else
	{
		return A;
	}
}

uint32_t FUNCTION_ATTRIBUTE net32_to_host(uint32_t A)
{
	if(is_little_endian())
	{
		return ((((A) & 0xff000000) >> 24) | \
			(((A) & 0x00ff0000) >> 8 ) | \
			(((A) & 0x0000ff00) << 8 ) | \
			(((A) & 0x000000ff) << 24));
	}
	else
	{
		return A;
	}
}

uint64_t FUNCTION_ATTRIBUTE net64_to_host(uint64_t A)
{
	uint64_t mask = 0xff;
	if(is_little_endian())
	{
		return 	((((A) & (mask << 56)) >> 56) | \
			(((A) & (mask << 48)) >> 40) | \
			(((A) & (mask << 40)) >> 24) | \
			(((A) & (mask << 32)) >> 8) | \
			(((A) & (mask << 24)) << 8) | \
			(((A) & (mask << 16)) << 24) | \
			(((A) & (mask << 8)) << 40) | \
			(((A) & (mask)) << 56));
	}
	else
	{
		return A;
	}
}


float FUNCTION_ATTRIBUTE net32f_to_host(float A)
{
	if(is_little_endian())
	{
		union float_int
		{
			float f;
			uint32_t i;
		}x;
		x.f = A;
		x.i = net32_to_host(x.i);
		return x.f;
	}
	else
	{
		return A;
	}
}

double FUNCTION_ATTRIBUTE net64f_to_host(double A)
{
	if(is_little_endian())
	{
		union double_uint64
		{
			double d;
			uint64_t i;
		}x;
		x.d = A;
		x.i = net64_to_host(x.i);
		return x.d;
	}
	else
	{
		return A;
	}
}


