
#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead;
//static int g_iDbgLevelLimit = 8;

//将DebugOpr放入链表中
int RegisterDebugOpr (PT_DebugOpr ptDebugOpr)
{
	PT_DebugOpr ptTmp;

	if (!g_ptDebugOprHead)
	{
		g_ptDebugOprHead = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}
	else 
	{
		ptTmp = g_ptDebugOprHead;

		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}

		ptTmp->ptNext = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}

	return 0;
}


//打印DebugOpr链表所有节点
void ShowDebugOpr (void)
{
	int i = 0;
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		printf ("%02d %s\n",i++,ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


//取出指定DebugOpr节点
PT_DebugOpr GetDebugOpr (char * pcName)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

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


int SetDbgLevel (char * strBuf)
{
  return 0;

}


/*
 * stdout=0			   : 关闭stdout打印
 * stdout=1			   : 打开stdout打印
 * netprint=0		   : 关闭netprint打印
 * netprint=1		   : 打开netprint打印
 */
int SetDbgChanel (char * strBuf)
{
	char * pStrTmp;
	char  StrName[20];
	PT_DebugOpr pt_tmp;


	//在字符串 strBuf 中查找字符'='，返回字符'='第一次在字符串strBuf中出现的位置（指针）
	pStrTmp = strchr (strBuf,'=');
	if (pStrTmp == NULL)
	{
		return - 1;
	}
	else 
	{
	    strncmp (StrName,strBuf,pStrTmp - strBuf); //获取名字
		StrName[pStrTmp - strBuf] = '0';

		pt_tmp = GetDebugOpr (StrName);
		if (pt_tmp)
		{
			pt_tmp->isCanUse = pStrTmp[1]-0x30;
		}
		
	}
	return 0;



}


int DebugPrint (const char * pcFormat,...)
{
	va_list tArg;
	char strTmpBuf[1000];
	int iNum;
	PT_DebugOpr ptTmp;
    //char strFileName[200];

	va_start (tArg,pcFormat);
	iNum = vsprintf (strTmpBuf,pcFormat,tArg);		//获取可变参数中的值
	va_end (tArg);
	strTmpBuf[iNum] = '\0';


	ptTmp = g_ptDebugOprHead;

	//执行debug链表中的打印语句
	while (ptTmp)
	{
		if ((ptTmp->DebugPrint !=NULL) && (ptTmp->isCanUse)) ptTmp->DebugPrint (strTmpBuf);

		ptTmp = ptTmp->ptNext;

	}

    return 0;
}


int DebugInit (void)
{
	int iError;

	iError = StdoutInit ();
	iError |= NetPrintInit ();
	return iError;
}


//设备初始化
int InitDebugChanel (void)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		if (ptTmp->isCanUse && ptTmp->DebugInit)
		{
			ptTmp->DebugInit ();
		}

		ptTmp = ptTmp->ptNext;
	}

	return 0;
}


