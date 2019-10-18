#include <config.h>
#include <video_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static int g_aiSupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};



//检查是否是此文件能支持的格式
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


//v4l2设备初始化
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
	int iFd;
	struct v4l2_capability tV4l2Cap;


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

	//查询支持哪种格式




	return 0;

err_exit:
	return -1;



}

static int V4l2ExitDevice ( PT_VideoDevice ptVideoDevice )
{}


static int V4l2GetFrameForStreaming ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{}

static int V4l2PutFrameForStreaming ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{}

static int V4l2GetFrameForReadWrite ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{}


static int V4l2PutFrameForReadWrite ( PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf )
{}

static int V4l2StartDevice ( PT_VideoDevice ptVideoDevice )
{

}

static int V4l2StopDevice ( PT_VideoDevice ptVideoDevice )
{}


static int V4l2GetFormat ( PT_VideoDevice ptVideoDevice )
{}

static int V4l2GetFormat ( PT_VideoDevice ptVideoDevice )
{}




T_VideoOpr g_tV4l2VideoOpr =
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


