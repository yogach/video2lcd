#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include <debug_manager.h>

#define FB_DEVICE_NAME "/dev/fb0"
#define DEFAULT_DIR "/"

#define COLOR_BACKGROUND   0xE7DBB5  /* ·º»ÆµÄÖ½ */
#define COLOR_FOREGROUND   0x514438  /* ºÖÉ«×ÖÌå */

  
#define DBG_PRINTF DebugPrint("%s[%d]:"fmt,__FILE__,__LINE__,##__VA_ARGS__)



#endif /* _CONFIG_H */
