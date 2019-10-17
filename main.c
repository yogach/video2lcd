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

	/* ��ʼ������ģ��: ����ͨ��"��׼���"Ҳ����ͨ��"����"��ӡ������Ϣ
	 * ��Ϊ�������Ͼ�Ҫ�õ�DBG_PRINTF����, �����ȳ�ʼ������ģ��
	 */

	/* ע�����ͨ�� */
	DebugInit();

	/* ��ʼ������ͨ�� */
	InitDebugChanel();

	if (argc != 2)
	{
		DBG_PRINTF("Usage:\n");
		DBG_PRINTF("%s <freetype_file>\n", argv[0]);
		return 0;
	}

	/* ע����ʾ�豸 */
	DisplayInit();
	/* ���ܿ�֧�ֶ����ʾ�豸: ѡ��ͳ�ʼ��ָ������ʾ�豸 */
	SelectAndInitDefaultDispDev("fb");

	
		
	return 0;
}
