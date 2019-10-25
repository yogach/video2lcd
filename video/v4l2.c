#include <config.h>
#include <video_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <disp_manager.h>
#include <sys/mman.h>
#include <stdlib.h>

//���ļ�֧�ֵ����ظ�ʽ
static int g_aiSupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};

static T_VideoOpr g_tV4l2VideoOpr;


//����Ƿ����豸֧�ֵĲ���ģʽ �Ƿ��Ǳ����֧�ֵ�
static int isSupportThisFormat ( int iPixelFormat )
{
	int i;

	for ( i=0; i< sizeof ( g_aiSupportedFormats ) /sizeof ( g_aiSupportedFormats[0] ); i++ )
	{

		if ( iPixelFormat == g_aiSupportedFormats[i] )
		{
			return 1;
		}
	}

	return 0;

}


static int V4l2GetFrameForStreaming ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{
	struct pollfd tFds[1];
	int iRet;
	struct v4l2_buffer tV4l2Buf;

	//ʹ��poll���Ƶȴ�Ӧ�ó���֪ͨ�ɶ�
	tFds[0].fd     = ptVideoDevice->iFd;
	tFds[0].events = POLLIN;

	iRet = poll ( tFds, 1, -1 ); //��ʱʱ������Ϊ-1 ������Զ�ȴ�
	if ( iRet <= 0 )
	{
		DBG_PRINTF ( "poll error!\n" );
		return -1;
	}

	// VIDIOC_DQBUF������ ʹ��ioctl���ж�ȡ
	memset ( &tV4l2Buf, 0, sizeof ( struct v4l2_buffer ) );
	tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	iRet = ioctl ( ptVideoDevice->iFd, VIDIOC_DQBUF, &tV4l2Buf );
	if ( iRet < 0 )
	{
		DBG_PRINTF ( "Unable to dequeue buffer.\n" );
		return -1;
	}

	ptVideoDevice->iVideoBufCurIndex = tV4l2Buf.index;

	ptVideoBuf->iPixelFormat = ptVideoDevice->iPixelFormat;
	ptVideoBuf->tPixelDatas.iHeight = ptVideoDevice->iHeight;
	ptVideoBuf->tPixelDatas.iWidth = ptVideoDevice->iWidth;

	//���ݲ�ͬ�����ظ�ʽ���ò�ͬ��bpp
	ptVideoBuf->tPixelDatas.iBpp = ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_YUYV ) ? 16:\
	                               ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_MJPEG ) ? 0:\
	                               ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_RGB565 ) ? 16:\
	                               0;

	ptVideoBuf->tPixelDatas.iLineBytes = ptVideoDevice->iWidth * ptVideoBuf->tPixelDatas.iBpp / 8;
	ptVideoBuf->tPixelDatas.iTotalBytes = tV4l2Buf.bytesused;
	ptVideoBuf->tPixelDatas.aucPixelDatas = ptVideoDevice->pucVideBuf[tV4l2Buf.index];

	return 0;

}

static int V4l2PutFrameForStreaming ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{

	struct v4l2_buffer tV4l2Buf;
	int iError;

	/* VIDIOC_QBUF videobuff����� */
	memset ( &tV4l2Buf, 0, sizeof ( struct v4l2_buffer ) );
	tV4l2Buf.index  = ptVideoDevice->iVideoBufCurIndex; //�����ǵڼ���buff���������
	tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	iError = ioctl ( ptVideoDevice->iFd, VIDIOC_QBUF, &tV4l2Buf );
	if ( iError )
	{
		DBG_PRINTF ( "Unable to queue buffer.\n" );
		return -1;
	}
	return 0;

}

static int V4l2GetFrameForReadWrite ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{
	int iRet;

	//ʹ��read������ȡ���� ����ֵΪ��ȡ�������ݳ���
	iRet = read ( ptVideoDevice->iFd, ptVideoDevice->pucVideBuf[0], ptVideoDevice->iVideoBufMaxLen );

	if ( iRet <= 0 )
	{
		DBG_PRINTF ( "read error!\r\n" );
		return -1;
	}

	ptVideoDevice->iVideoBufCurIndex = 0;

	ptVideoBuf->iPixelFormat = ptVideoDevice->iPixelFormat;
	ptVideoBuf->tPixelDatas.iHeight = ptVideoDevice->iHeight;
	ptVideoBuf->tPixelDatas.iWidth = ptVideoDevice->iWidth;

	//���ݲ�ͬ�����ظ�ʽ���ò�ͬ��bpp
	ptVideoBuf->tPixelDatas.iBpp = ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_YUYV ) ? 16:\
	                               ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_MJPEG ) ? 0:\
	                               ( ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_RGB565 ) ? 16:\
	                               0;

	ptVideoBuf->tPixelDatas.iLineBytes = ptVideoDevice->iWidth * ptVideoBuf->tPixelDatas.iBpp / 8;
	ptVideoBuf->tPixelDatas.iTotalBytes = iRet;
	ptVideoBuf->tPixelDatas.aucPixelDatas = ptVideoDevice->pucVideBuf[0];


	return 0;

}


static int V4l2PutFrameForReadWrite ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{

	return 0;

}

static int V4l2StartDevice ( PT_VideoDevice ptVideoDevice )
{

    int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iError;

	//���豸����Ƶ��׽ģʽ
    iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMON, &iType);
    if (iError) 
    {
    	DBG_PRINTF("Unable to start capture.\n");
    	return -1;
    }
    return 0;


}

static int V4l2StopDevice ( PT_VideoDevice ptVideoDevice )
{
    int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iError;

	//�ر��豸����Ƶ��׽ģʽ
    iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMOFF, &iType);
    if (iError) 
    {
    	DBG_PRINTF("Unable to stop capture.\n");
    	return -1;
    }

	return 0;

}


static int V4l2GetFormat ( PT_VideoDevice ptVideoDevice )
{
    return ptVideoDevice->iPixelFormat; //�����豸֧�ֵĲ���ģʽ
}


//v4l2�豸��ʼ�� --�����豸�� ---�����豸��������
/* open
 * VIDIOC_QUERYCAP ȷ�����Ƿ���Ƶ��׽�豸,֧�����ֽӿ�(streaming/read,write)
 * VIDIOC_ENUM_FMT ��ѯ֧�����ָ�ʽ
 * VIDIOC_S_FMT    ��������ͷʹ�����ָ�ʽ
 * VIDIOC_REQBUFS  ����buffer
 ���� streaming�ӿ�:
 * VIDIOC_QUERYBUF ȷ��ÿһ��buffer����Ϣ ���� mmap
 * VIDIOC_QBUF     �������
 * VIDIOC_STREAMON �����豸
 * poll            �ȴ�������
 * VIDIOC_DQBUF    �Ӷ�����ȡ��
 * ����....
 * VIDIOC_QBUF     �������
 * ....
 ����read,write�ӿ�:
    read
    ����....
    read
 * VIDIOC_STREAMOFF ֹͣ�豸
 *
 */
static int V4l2InitDevice ( char* strDevName, PT_VideoDevice ptVideoDevice )
{
	int iFd,iError;
	struct v4l2_capability tV4l2Cap;
	struct v4l2_fmtdesc tFmtDesc;
	struct v4l2_format	tV4l2Fmt;
	struct v4l2_requestbuffers tV4l2ReqBuffs;
	struct v4l2_buffer tV4l2Buf;
	int iLcdWidth, iLcdHeigt, iLcdBpp ,i;

	iFd = open ( strDevName, O_RDWR ); //�Կɶ���д��ʽ���豸
	if ( iFd < 0 )
	{
		DBG_PRINTF ( "can't open this device:%s",strDevName );
		goto err_exit;
	}

	ptVideoDevice->iFd =iFd;

	//����豸֧�ֵĽӿ�
	//iError = ioctl ( iFd, VIDIOC_QUERYCAP, &tV4l2Cap );
	memset ( &tV4l2Cap, 0, sizeof ( struct v4l2_capability ) );
	iError = ioctl ( iFd, VIDIOC_QUERYCAP, &tV4l2Cap );
	if ( iError )
	{
		DBG_PRINTF ( "Error opening device %s: unable to query device.\n", strDevName );
		goto err_exit;
	}

	//������豸����һ�� VIDEO_CAPTURE�豸
	if ( ! ( tV4l2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) )
	{
		DBG_PRINTF ( "%s is not a video capture device\n", strDevName );
		goto err_exit;
	}
	if ( tV4l2Cap.capabilities & V4L2_CAP_STREAMING )
	{
		DBG_PRINTF ( "%s supports streaming i/o\n", strDevName );
	}
	if ( tV4l2Cap.capabilities & V4L2_CAP_READWRITE )
	{
		DBG_PRINTF ( "%s supports read i/o\n", strDevName );
	}

	ptVideoDevice->iVideoCapabilities = tV4l2Cap.capabilities;

	//��ѯ�豸֧�ֲ���ģʽ
	memset ( &tFmtDesc, 0, sizeof ( struct v4l2_fmtdesc ) );
	tFmtDesc.index = 0;
	tFmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ( ( iError = ioctl ( iFd, VIDIOC_ENUM_FMT, &tFmtDesc ) ) == 0 )
	{
		if ( isSupportThisFormat ( tFmtDesc.pixelformat ) )
		{
			ptVideoDevice->iPixelFormat = tFmtDesc.pixelformat;
			break;
		}
		tFmtDesc.index++;
	}

	if ( !ptVideoDevice->iPixelFormat ) //���δ��ȡ���豸֧�ֵĲ���ģʽ ����
	{
		DBG_PRINTF ( "can not support the format of this device\n" );
		goto err_exit;
	}

	/* set format in */
	//������buffer֮ǰ �������ú�buffer������
	GetDispResolution ( &iLcdWidth, &iLcdHeigt, &iLcdBpp ); //��ȡ�ֱ���
	memset ( &tV4l2Fmt, 0, sizeof ( struct v4l2_format ) );
	tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Fmt.fmt.pix.pixelformat = ptVideoDevice->iPixelFormat;
	//���÷ֱ���
	tV4l2Fmt.fmt.pix.width		= iLcdWidth;
	tV4l2Fmt.fmt.pix.height		= iLcdHeigt;
	tV4l2Fmt.fmt.pix.field		= V4L2_FIELD_ANY; //���豸����ѡ��


	/* �������豸������������
	 * ��������������޷�ĳЩ����(����ֱ���),
	 * ���������Щ����, ���ҷ��ظ�Ӧ�ó���
	 */
	iError = ioctl ( iFd, VIDIOC_S_FMT, &tV4l2Fmt );
	if ( iError )
	{
		DBG_PRINTF ( "Unable to set format\n" );
		goto err_exit;
	}
	//�������÷ֱ���
	ptVideoDevice->iWidth  = tV4l2Fmt.fmt.pix.width;
	ptVideoDevice->iHeight = tV4l2Fmt.fmt.pix.height;

	//����buffer
	memset ( &tV4l2ReqBuffs, 0, sizeof ( struct v4l2_requestbuffers ) );
	tV4l2ReqBuffs.count = NB_BUFFER;                 //�����buff����
	tV4l2ReqBuffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//����
	tV4l2ReqBuffs.memory = V4L2_MEMORY_MMAP;

	iError = ioctl ( iFd, VIDIOC_REQBUFS, &tV4l2ReqBuffs );
	if ( iError )
	{
		DBG_PRINTF ( "Unable to allocate buffers.\n" );
		goto err_exit;
	}

	ptVideoDevice->iVideoBufCnt = tV4l2ReqBuffs.count;
	if ( tV4l2Cap.capabilities & V4L2_CAP_STREAMING ) 
	{
	    
		/*����豸֧�ֵ�V4L2_CAP_STREAMING����ģʽ
         *����ʹ VIDIOC_QUERYBUF ������
         *ʹ��mmap������ӳ�䵽�ڴ���
		 *Ȼ��ʹ��VIDIOC_QBUF���������
		 */

		//map the buffers
		for ( i = 0; i < ptVideoDevice->iVideoBufCnt; i++ )
		{
			memset ( &tV4l2Buf, 0, sizeof ( struct v4l2_buffer ) );
			tV4l2Buf.index = i;
			tV4l2Buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			tV4l2Buf.memory = V4L2_MEMORY_MMAP;
			iError = ioctl ( iFd, VIDIOC_QUERYBUF, &tV4l2Buf );
			if ( iError )
			{
				DBG_PRINTF ( "Unable to query buffer.\n" );
				goto err_exit;
			}

			ptVideoDevice->iVideoBufMaxLen = tV4l2Buf.length;
			//��buf���ݵ�ַmmap��������
			ptVideoDevice->pucVideBuf[i] = mmap ( 0 /* start anywhere */,
			                                      tV4l2Buf.length, PROT_READ, MAP_SHARED, iFd,
			                                      tV4l2Buf.m.offset );
			if ( ptVideoDevice->pucVideBuf[i] == MAP_FAILED )
			{
				DBG_PRINTF ( "Unable to map buffer\n" );
				goto err_exit;
			}

		}


		/* Queue the buffers. */
		for ( i = 0; i < ptVideoDevice->iVideoBufCnt; i++ )
		{
			memset ( &tV4l2Buf, 0, sizeof ( struct v4l2_buffer ) );
			tV4l2Buf.index = i;
			tV4l2Buf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			tV4l2Buf.memory = V4L2_MEMORY_MMAP;
			iError = ioctl ( iFd, VIDIOC_QBUF, &tV4l2Buf );
			if ( iError )
			{
				DBG_PRINTF ( "Unable to queue buffer.\n" );
				goto err_exit;
			}

		}

	}
	else if ( tV4l2Cap.capabilities & V4L2_CAP_READWRITE ) 
	{

	    /*����豸֧�ֵ�V4L2_CAP_READWRITE����ģʽ
         * ֱ��mallocһ���ռ�
	     */ 
		g_tV4l2VideoOpr.GetFrame =V4l2GetFrameForReadWrite;
		g_tV4l2VideoOpr.PutFrame =V4l2PutFrameForReadWrite;

		//����һ��videobuf�ռ�
		ptVideoDevice->iVideoBufCnt  = 1;
		/* �������������֧�ֵĸ�ʽ��, һ���������ֻ��Ҫ4�ֽ� */
		ptVideoDevice->iVideoBufMaxLen = ptVideoDevice->iWidth * ptVideoDevice->iHeight * 4;
		ptVideoDevice->pucVideBuf[0] = malloc ( ptVideoDevice->iVideoBufMaxLen );

	}

	ptVideoDevice->ptOPr = &g_tV4l2VideoOpr;



	return 0;

err_exit:


	close ( iFd ); // �ر��豸
	return -1;



}

static int V4l2ExitDevice ( PT_VideoDevice ptVideoDevice )
{
	int i;
	//����֧�ֽӿڸ�ʽ�Ĳ�ͬ
	if ( ptVideoDevice->iVideoCapabilities  & V4L2_CAP_STREAMING )
	{
		//�ͷ�map�Ŀռ�
		for ( i=0; i<ptVideoDevice->iVideoBufCnt; i++ )
		{
			if ( ptVideoDevice->pucVideBuf[i] )
			{
				munmap ( ptVideoDevice->pucVideBuf,ptVideoDevice->iVideoBufMaxLen );
				ptVideoDevice->pucVideBuf[i] = NULL;
			}

		}
	}
	else if ( ptVideoDevice->iVideoCapabilities & V4L2_CAP_READWRITE )
	{

		free ( ptVideoDevice->pucVideBuf[0] );
		ptVideoDevice->pucVideBuf[0] =NULL;

	}
	close ( ptVideoDevice->iFd ); // �ر��豸
	return 0;
}


static T_VideoOpr g_tV4l2VideoOpr =
{
	.name        = "v4l2",
	.InitDevice  = V4l2InitDevice,
	.ExitDevice  = V4l2ExitDevice,
	.GetFormat   = V4l2GetFormat,
	.GetFrame    = V4l2GetFrameForStreaming,
	.PutFrame    = V4l2PutFrameForStreaming,
	.StartDevice = V4l2StartDevice,
	.StopDevice  = V4l2StopDevice,
};


int V4l2Init ( void )
{
	return RegisterVideoOpr ( &g_tV4l2VideoOpr );
}


