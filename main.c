#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <pic_operation.h>
#include <render.h>
#include <string.h>
#include <picfmt_manager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


/* digitpic <freetype_file> */
int main(int argc, char **argv)
{	
	int iError;

	/* 初始化调试模块: 可以通过"标准输出"也可以通过"网络"打印调试信息
	 * 因为下面马上就要用到DBG_PRINTF函数, 所以先初始化调试模块
	 */

	/* 注册调试通道 */
	DebugInit();

	/* 初始化调试通道 */
	InitDebugChanel();

	if (argc != 2)
	{
		DBG_PRINTF("Usage:\n");
		DBG_PRINTF("%s <freetype_file>\n", argv[0]);
		return 0;
	}

	/* 注册显示设备 */
	DisplayInit();
	/* 可能可支持多个显示设备: 选择和初始化指定的显示设备 */
	SelectAndInitDefaultDispDev("fb");

	
		
	return 0;
}

