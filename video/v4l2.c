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

//此文件支持的像素格式
static int g_aiSupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};

static T_VideoOpr g_tV4l2VideoOpr;


//检查是否是设备支持的操作模式 是否是本软件支持的
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

	//使用poll机制等待应用程序通知可读
	tFds[0].fd     = ptVideoDevice->iFd;
	tFds[0].events = POLLIN;

	iRet = poll ( tFds, 1, -1 ); //超时时间设置为-1 代表永远等待
	if ( iRet <= 0 )
	{
		DBG_PRINTF ( "poll error!\n" );
		return -1;
	}

	// VIDIOC_DQBUF出队列 使用ioctl进行读取
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

	//根据不同的像素格式设置不同的bpp
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

	/* VIDIOC_QBUF videobuff入队列 */
	memset ( &tV4l2Buf, 0, sizeof ( struct v4l2_buffer ) );
	tV4l2Buf.index  = ptVideoDevice->iVideoBufCurIndex; //设置是第几个buff重新入队列
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

	//使用read操作读取数据 返回值为读取到的数据长度
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

	//根据不同的像素格式设置不同的bpp
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

	//打开设备的视频捕捉模式
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

	//关闭设备的视频捕捉模式
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
    return ptVideoDevice->iPixelFormat; //返回设备支持的操作模式
}


//v4l2设备初始化 --传入设备名 ---传出设备操作函数
/* open
 * VIDIOC_QUERYCAP 确定它是否视频捕捉设备,支持哪种接口(streaming/read,write)
 * VIDIOC_ENUM_FMT 查询支持哪种格式
 * VIDIOC_S_FMT    设置摄像头使用哪种格式
 * VIDIOC_REQBUFS  申请buffer
 对于 streaming接口:
 * VIDIOC_QUERYBUF 确定每一个buffer的信息 并且 mmap
 * VIDIOC_QBUF     放入队列
 * VIDIOC_STREAMON 启动设备
 * poll            等待有数据
 * VIDIOC_DQBUF    从队列中取出
 * 处理....
 * VIDIOC_QBUF     放入队列
 * ....
 对于read,write接口:
    read
    处理....
    read
 * VIDIOC_STREAMOFF 停止设备
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

	iFd = open ( strDevName, O_RDWR ); //以可读可写方式打开设备
	if ( iFd < 0 )
	{
		DBG_PRINTF ( "can't open this device:%s",strDevName );
		goto err_exit;
	}

	ptVideoDevice->iFd =iFd;

	//获得设备支持的接口
	//iError = ioctl ( iFd, VIDIOC_QUERYCAP, &tV4l2Cap );
	memset ( &tV4l2Cap, 0, sizeof ( struct v4l2_capability ) );
	iError = ioctl ( iFd, VIDIOC_QUERYCAP, &tV4l2Cap );
	if ( iError )
	{
		DBG_PRINTF ( "Error opening device %s: unable to query device.\n", strDevName );
		goto err_exit;
	}

	//如果该设备不是一个 VIDEO_CAPTURE设备
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

	//查询设备支持操作模式
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

	if ( !ptVideoDevice->iPixelFormat ) //如果未获取到设备支持的操作模式 跳出
	{
		DBG_PRINTF ( "can not support the format of this device\n" );
		goto err_exit;
	}

	/* set format in */
	//在申请buffer之前 首先设置好buffer的属性
	GetDispResolution ( &iLcdWidth, &iLcdHeigt, &iLcdBpp ); //获取分辨率
	memset ( &tV4l2Fmt, 0, sizeof ( struct v4l2_format ) );
	tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Fmt.fmt.pix.pixelformat = ptVideoDevice->iPixelFormat;
	//设置分辨率
	tV4l2Fmt.fmt.pix.width		= iLcdWidth;
	tV4l2Fmt.fmt.pix.height		= iLcdHeigt;
	tV4l2Fmt.fmt.pix.field		= V4L2_FIELD_ANY; //由设备自行选择


	/* 将参数设备到驱动程序中
	 * 如果驱动程序发现无法某些参数(比如分辨率),
	 * 它会调整这些参数, 并且返回给应用程序
	 */
	iError = ioctl ( iFd, VIDIOC_S_FMT, &tV4l2Fmt );
	if ( iError )
	{
		DBG_PRINTF ( "Unable to set format\n" );
		goto err_exit;
	}
	//重新设置分辨率
	ptVideoDevice->iWidth  = tV4l2Fmt.fmt.pix.width;
	ptVideoDevice->iHeight = tV4l2Fmt.fmt.pix.height;

	//申请buffer
	memset ( &tV4l2ReqBuffs, 0, sizeof ( struct v4l2_requestbuffers ) );
	tV4l2ReqBuffs.count = NB_BUFFER;                 //分配的buff数量
	tV4l2ReqBuffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型
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
	    
		/*如果设备支持的V4L2_CAP_STREAMING操作模式
         *首先使 VIDIOC_QUERYBUF 出队列
         *使用mmap将数据映射到内存上
		 *然后使用VIDIOC_QBUF重新入队列
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
			//将buf数据地址mmap到数组中
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

	    /*如果设备支持的V4L2_CAP_READWRITE操作模式
         * 直接malloc一个空间
	     */ 
		g_tV4l2VideoOpr.GetFrame =V4l2GetFrameForReadWrite;
		g_tV4l2VideoOpr.PutFrame =V4l2PutFrameForReadWrite;

		//分配一个videobuf空间
		ptVideoDevice->iVideoBufCnt  = 1;
		/* 在这个程序所能支持的格式里, 一个象素最多只需要4字节 */
		ptVideoDevice->iVideoBufMaxLen = ptVideoDevice->iWidth * ptVideoDevice->iHeight * 4;
		ptVideoDevice->pucVideBuf[0] = malloc ( ptVideoDevice->iVideoBufMaxLen );

	}

	ptVideoDevice->ptOPr = &g_tV4l2VideoOpr;



	return 0;

err_exit:


	close ( iFd ); // 关闭设备
	return -1;



}

static int V4l2ExitDevice ( PT_VideoDevice ptVideoDevice )
{
	int i;
	//根据支持接口格式的不同
	if ( ptVideoDevice->iVideoCapabilities  & V4L2_CAP_STREAMING )
	{
		//释放map的空间
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
	close ( ptVideoDevice->iFd ); // 关闭设备
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


