#ifndef _SHARED_H_
#define _SHARED_H_

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

typedef signed char int8;
typedef signed short int int16;
typedef signed long int int32;

#ifdef NGC
#include "osd.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>
//#include <zlib.h>

#define ESP32_PSRAM (0x3f800000)

#ifndef NGC
#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#else
#define PATH_MAX 1024
#endif
#endif
#endif

#include "cpu/z80.h"
#include "sms.h"
#include "pio.h"
#include "memz80.h"
#include "vdp.h"
#include "render.h"
#include "tms.h"
#include "sound/sn76489.h"
#include "sound/emu2413.h"
#include "sound/ym2413.h"
#include "sound/fmintf.h"
#include "sound/sound.h"
#include "system.h"
#include "error.h"
#include "loadrom.h"
#include "config.h"
#include "state.h"

#ifndef NGC
#include "fileio.h"
#endif

#endif /* _SHARED_H_ */
