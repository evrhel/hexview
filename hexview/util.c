#include "util.h"

#include <stdio.h>
#include <memory.h>

int
to_native_endianess(const value_u *in, int max_read, int endianess, outvalues_t *const out)
{
#if NATIVE_ENDIANESS == 0
	int toread;

	toread = max_read < sizeof(value_u) ? max_read : sizeof(value_u);

	memset(out, 0, sizeof(outvalues_t));
	out->utf8 = (utf8_t *)in;

	switch (endianess)
	{
	case LittleEndian:
		if (max_read < 1) break;
		out->ui8 = in->ui8;

		if (max_read < 2) break;
		out->ui16 = in->ui16;

		if (max_read < 4) break;
		out->ui32 = in->ui32;

		if (max_read < 8) break;
		out->ui64 = in->ui64;

		break;
	case BigEndian:
		if (max_read < 1) break;
		out->ui8 = in->ui8;

		if (max_read < 2) break;
		out->ui16 = swap_endianess16(in->ui16);

		if (max_read < 4)
		{
			max_read = 2;
			break;
		}
		out->ui32 = swap_endianess32(in->ui32);

		if (max_read < 8)
		{
			max_read = 4;
			break;
		}
		out->ui64 = swap_endianess64(in->ui64);

		break;
	}

	return max_read;
#elif NATIVE_ENDIANESS == 1
	// not supported
#endif
}

int
readline(char *const out, int maxcount)
{
	char c;
	int off;

	for (off = 0; off < maxcount; off++)
	{
		c = fgetc(stdin);
		if (c == '\n')
		{
			out[off] = 0;
			break;
		}
		out[off] = c;
	}

	return off;
}