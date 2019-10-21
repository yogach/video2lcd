#include <config.h>
#include <convert_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_VideoConvert g_ptVideoConvertOprHead;


//��VideoConvertOpr����������
int RegisterVideoConvertOpr (PT_VideoConvert ptVideoConvertOpr)
{
	PT_VideoConvert ptTmp;

	if (!g_ptVideoConvertOprHead)
	{
		g_ptVideoConvertOprHead = ptVideoConvertOpr;
		ptVideoConvertOpr->ptNext = NULL;
	}
	else 
	{
		ptTmp = g_ptVideoConvertOprHead;

		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}

		ptTmp->ptNext = ptVideoConvertOpr;
		ptVideoConvertOpr->ptNext = NULL;
	}

	return 0;
}


//��ӡVideoConvertOpr�������нڵ�
void ShowVideoConvertOpr (void)
{
	int i = 0;
	PT_VideoConvert ptTmp = g_ptVideoConvertOprHead;

	while (ptTmp)
	{
		printf ("%02d %s\n",i++,ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


//ȡ��ָ��VideoConvertOpr�ڵ�
PT_VideoConvert GetVideoConvertOpr (char * pcName)
{
	PT_VideoConvert ptTmp = g_ptVideoConvertOprHead;

	while (ptTmp)
	{
		if (strcmp (ptTmp->name,pcName) == 0)
		{
			return ptTmp;
		}

		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}


int VideoConvertInit (void)
{
	int iError;
	iError = Rgb2RgbConvertInit();
	iError |= Yuv2RgbConvertInit();
	iError |= Mjpeg2RgbConvertInit();
	
    
	return iError;
}

