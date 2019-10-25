#include <config.h>
#include <video_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_VideoOpr g_ptVideoOprHead;


//将VideoOpr放入链表中
int RegisterVideoOpr (PT_VideoOpr ptVideoOpr)
{
	PT_VideoOpr ptTmp;

	if (!g_ptVideoOprHead)
	{
		g_ptVideoOprHead = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}
	else 
	{
		ptTmp = g_ptVideoOprHead;

		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}

		ptTmp->ptNext = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}

	return 0;
}


//打印VideoOpr链表所有节点
void ShowVideoOpr (void)
{
	int i = 0;
	PT_VideoOpr ptTmp = g_ptVideoOprHead;

	while (ptTmp)
	{
		printf ("%02d %s\n",i++,ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


//取出指定VideoOpr节点
PT_VideoOpr GetVideoOpr (char * pcName)
{
	PT_VideoOpr ptTmp = g_ptVideoOprHead;

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


int	 VideoDeviceInit(char *DeviceName, PT_VideoDevice ptVideoDevice)
{
   int iError;
   
   PT_VideoOpr ptTmp = g_ptVideoOprHead;

   //遍历链表 调用初始化函数 如果初始化成功 返回成功
   while (ptTmp)
   {
       iError = ptTmp->InitDevice(DeviceName,ptVideoDevice);
	   if (!iError)
	   {
		   return 0;
	   }
   
	   ptTmp = ptTmp->ptNext;
   }

  return -1;
}

int VideoInit (void)
{
	int iError;

	iError = V4l2Init();

    
	return iError;
}


