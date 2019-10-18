#include <config.h>
#include <video_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static int g_aiSupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};



//����Ƿ��Ǵ��ļ���֧�ֵĸ�ʽ
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


//v4l2�豸��ʼ��
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
	int iFd;
	struct v4l2_capability tV4l2Cap;


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

	//��ѯ֧�����ָ�ʽ




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


