
#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead;
//static int g_iDbgLevelLimit = 8;

//��DebugOpr����������
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


//��ӡDebugOpr�������нڵ�
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


//ȡ��ָ��DebugOpr�ڵ�
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
 * stdout=0			   : �ر�stdout��ӡ
 * stdout=1			   : ��stdout��ӡ
 * netprint=0		   : �ر�netprint��ӡ
 * netprint=1		   : ��netprint��ӡ
 */
int SetDbgChanel (char * strBuf)
{
	char * pStrTmp;
	char  StrName[20];
	PT_DebugOpr pt_tmp;


	//���ַ��� strBuf �в����ַ�'='�������ַ�'='��һ�����ַ���strBuf�г��ֵ�λ�ã�ָ�룩
	pStrTmp = strchr (strBuf,'=');
	if (pStrTmp == NULL)
	{
		return - 1;
	}
	else 
	{
	    strncmp (StrName,strBuf,pStrTmp - strBuf); //��ȡ����
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
	iNum = vsprintf (strTmpBuf,pcFormat,tArg);		//��ȡ�ɱ�����е�ֵ
	va_end (tArg);
	strTmpBuf[iNum] = '\0';


	ptTmp = g_ptDebugOprHead;

	//ִ��debug�����еĴ�ӡ���
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


//�豸��ʼ��
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


