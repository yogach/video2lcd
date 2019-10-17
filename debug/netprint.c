
#include <config.h>
#include <debug_manager.h>
#include <sys/types.h>					/* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#define SERVER_PORT 			5678
#define PRINT_BUF_SIZE			(16*1024)

typedef struct PrintBuff
{
unsigned char * Buff;
unsigned int	ReadPos;
unsigned int	WritePos;

} T_PrintBuff;


static T_PrintBuff g_tPrintBuff;

static int g_iSocketServer;
static int g_iHaveConnet;

static struct sockaddr_in g_tSocketServerAddr; //���������IP
static struct sockaddr_in g_tSocketClientAddr; //����ͻ���ip 
static pthread_t g_tSendTreadID;
static pthread_t g_tRecvTreadID;


static pthread_mutex_t g_tNetDbgSendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_tNetDbgSendConVar = PTHREAD_COND_INITIALIZER;


//�жϻ������Ƿ�����
static int isFull (void)
{
	if ((g_tPrintBuff.WritePos + 1) % PRINT_BUF_SIZE == g_tPrintBuff.ReadPos)
		return 1;

	else 
		return 0;

}


//�������Ƿ�Ϊ��
static int isEmpty (void)
{
	if (g_tPrintBuff.ReadPos == g_tPrintBuff.WritePos)
		return 1;

	else 
		return 0;

}


static int PutData (char cVal)
{
	if (isFull ())
	{
		return - 1;
	}
	else 
	{
		g_tPrintBuff.Buff[g_tPrintBuff.WritePos] = cVal;

		//��������������󳤶�ʱ����0
		g_tPrintBuff.WritePos = (g_tPrintBuff.WritePos + 1) % PRINT_BUF_SIZE;
		return 0;
	}
}


static int GetData (char * pcVal)
{
	if (isEmpty ())
	{
		return - 1;
	}
	else 
	{
		*pcVal = g_tPrintBuff.Buff[g_tPrintBuff.ReadPos];

		g_tPrintBuff.ReadPos = (g_tPrintBuff.ReadPos + 1) % PRINT_BUF_SIZE;
		return 0;
	}


}


static void * NetDbgSendTreadFunction (void * pVoid)
{
	int iSendLen,i;
	int iAddrLen;
	char strTmpBuf[512];
	char tmpchar;

	while (1)
	{
		//�����߳�
		pthread_mutex_lock (&g_tNetDbgSendMutex);
		pthread_cond_wait (&g_tNetDbgSendConVar,&g_tNetDbgSendMutex);
		pthread_mutex_unlock (&g_tNetDbgSendMutex);

		while (g_iHaveConnet && !isEmpty ()) //���������е�����ȫ����ӡ���ͻ�����
		{
			i	= 0;

			while ((i < 512) && (0 == GetData (&tmpchar)))
			{
				strTmpBuf[i] = tmpchar;
				i++;
			}

			//��ͻ��˷�������
			iAddrLen = sizeof(struct sockaddr);
			iSendLen = sendto (g_iSocketServer,strTmpBuf,i,0,
				(const struct sockaddr *) &g_tSocketClientAddr,iAddrLen);



		}


	}

}


static void * NetDbgRecvTreadFunction (void * pVoid)
{
	socklen_t iAddrLen;
	int iRecvLen;

	struct sockaddr_in tSocketClientAddr;
	char ucRecvBuf[1000];


	while (1)
	{
		iAddrLen = sizeof (struct sockaddr);

		//���տͻ�����Ϣ
		iRecvLen = recvfrom (g_iSocketServer,ucRecvBuf,999,0,(struct sockaddr *) &tSocketClientAddr,&iAddrLen);

		if (iRecvLen > 0)
		{

			ucRecvBuf[iRecvLen] = '\0'; 			//���ý�����

			/*�ͻ��˷���setclient���ÿ�ʼ����*/
			if (strcmp (ucRecvBuf,"setclient") == 0)
			{
				g_tSocketClientAddr = tSocketClientAddr; //�õ��ͻ���IP
				g_iHaveConnet = 1;					//�пͻ���������

	 			iAddrLen = sizeof(struct sockaddr);
				sendto (g_iSocketServer,"revcmd",6,0,
					(const struct sockaddr *) &g_tSocketClientAddr,iAddrLen);
				
			}
			else 
			{
                SetDbgChanel(ucRecvBuf);
			}


		}


	}
}


static int NetDbgInit (void)
{
	int iRet;
	int opt = 1; 

	g_iSocketServer = socket (AF_INET,SOCK_DGRAM,0); //����ΪUDP����

	if (g_iSocketServer == -1)
	{
		printf ("socket init error\r\n");
		return - 1;

	}

	g_tSocketServerAddr.sin_family = AF_INET;
	g_tSocketServerAddr.sin_port = htons (SERVER_PORT); /* host to net, short */
	g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset (g_tSocketServerAddr.sin_zero,0,8);


    //ʹ��setsockopt�������Ա�֤�˿ڿɱ��ظ���
    iRet = setsockopt(g_iSocketServer, SOL_SOCKET,SO_REUSEADDR,   
    				(const void *)&opt, sizeof(opt) );	
	if (-1 == iRet)
	{
		printf("set sock option error!\n");
		close(g_iSocketServer);
		return -1;
	}


	//��socket��˿�
	iRet = bind (g_iSocketServer,(const struct sockaddr *) &g_tSocketServerAddr,
		sizeof (struct sockaddr));

	if (-1 == iRet)
	{
		printf ("bind error!\n");
		return - 1;
	}

	g_tPrintBuff.Buff = malloc (PRINT_BUF_SIZE);

	if (g_tPrintBuff.Buff == NULL)
	{
		printf ("malloc printfbuff error!\n");
		close (g_iSocketServer);
		return - 1;
	}

	//�������ͼ������߳�
	pthread_create (&g_tSendTreadID,NULL,NetDbgSendTreadFunction,NULL);
	pthread_create (&g_tRecvTreadID,NULL,NetDbgRecvTreadFunction,NULL);

	return 0;

}


static int NetDbgExit (void)
{
	/* �ر�socket,... */
	close (g_iSocketServer);
	free (g_tPrintBuff.Buff);//�ͷŷ���Ŀռ�
	return 0;
}


//��ӡ����-�Ƚ���ӡ���ݷ��뻺������
static int NetDbgPrint (char * strData)
{
	int i ;

	for ( i= 0; i < strlen (strData); i++)
	{
		if (PutData (strData[i]) == -1)
			break;//return - 1; //����������
	}

	//ÿ���������ݴ�ӡʱ������һ�η����̣߳�����Ƿ��пͻ�������
	pthread_mutex_lock (&g_tNetDbgSendMutex);
	pthread_cond_signal (&g_tNetDbgSendConVar);
	pthread_mutex_unlock (&g_tNetDbgSendMutex);
   return 0;

}


static T_DebugOpr g_tNetDbgOpr =
{
	.name = "netprint",
	.isCanUse = 1,
	.DebugInit = NetDbgInit,
	.DebugExit = NetDbgExit,
	.DebugPrint = NetDbgPrint,
};


int NetPrintInit (void)
{
	return RegisterDebugOpr (&g_tNetDbgOpr);
}


