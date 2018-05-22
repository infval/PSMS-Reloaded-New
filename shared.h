
#ifndef _SHARED_H_
#define _SHARED_H_

#define VERSION     "0.9.3"

#include <tamtypes.h>

/* Data types */
typedef u32 dword;
typedef u16 word;
typedef u8 byte;

enum {
    SRAM_SAVE   = 0,
    SRAM_LOAD   = 1
};

/* To keep the MAME code happy */
#define HAS_YM3812  1
typedef signed short int FMSAMPLE;

#include <malloc.h>
//#include <math.h>
#include <stdio.h>
#include <string.h>

#include "z80.h"
#include "sms.h"
#include "vdp.h"
#include "render.h"
#include "sn76496.h"
#include "fmopl.h"
#include "ym2413.h"
#include "system.h"

#endif /* _SHARED_H_ */

