#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>

#include <disp_manager.h>
#include <video_manager.h>
#include <pic_operation.h>
#include <render.h>
#include <string.h>
#include <convert_manager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>



/* video <video_device> */
int main ( int argc, char** argv )
{
	int iError;
	int iLcdWidth,iLcdHight,iLcdBpp;
	int iPixelFormatOfDisp, iPixelFormatOfVideo;
	T_VideoBuf tFrameBuf;
	T_VideoDevice tVideoDevice;
	PT_VideoConvert ptVideoConvert;

	T_VideoBuf tVideoBuf; //用于存储video原始设备
	T_VideoBuf tConvertBuf; //用于存储转化后的video数据
	T_VideoBuf tZoomBuf;//用于存储缩放后的video数据
	PT_VideoBuf ptVideoBufCur;
    float k;

    int iTopLeftX,iTopLeftY;
	

	if ( argc != 2 )
	{
		DBG_PRINTF ( "Usage:\n" );
		DBG_PRINTF ( "%s </dev/video0,1,...>\n", argv[0] );
		return 0;
	}

	/* 注册显示设备 */
	DisplayInit();

	SelectAndInitDefaultDispDev ( "fb" );
	GetDispResolution ( &iLcdWidth, &iLcdHight, &iLcdBpp );
	//获取lcd信息
	GetVideoBufForDisplay ( &tFrameBuf );
	iPixelFormatOfDisp = tFrameBuf.iPixelFormat;

	//初始化video设备
	VideoInit();

	//初始化命令行指定的video设备 tVideoDevice内包含video设备属性以及操作函数
	iError = VideoDeviceInit ( argv[1], &tVideoDevice );
	if ( iError != 0 )
	{
		DBG_PRINTF ( "VideoDeviceInit error\r\n" );
		return -1;
	}

	iPixelFormatOfVideo = tVideoDevice.iPixelFormat;

	//video格式转化初始化
	VideoConvertInit();

	//获得能够支持的输入输出 像素格式转换函数
	ptVideoConvert =  GetVideoConvertForFormats ( iPixelFormatOfVideo, iPixelFormatOfDisp );
	if ( ptVideoConvert == NULL )
	{

		DBG_PRINTF ( "can't support this Formats convert \r\n" );
		return -1;

	}


	/* 启动摄像头设备 */
	iError = tVideoDevice.ptOPr->StartDevice ( &tVideoDevice );
	if ( iError )
	{

		DBG_PRINTF ( "StartDevice for %s error!\n", argv[1] );
		return -1;
	}

	memset ( &tVideoBuf, 0, sizeof ( tVideoBuf ) );
	memset ( &tConvertBuf, 0, sizeof ( tConvertBuf ) );
	tConvertBuf.iPixelFormat = iPixelFormatOfDisp;
	tConvertBuf.tPixelDatas.iBpp = iLcdBpp;

	memset ( &tZoomBuf, 0, sizeof ( tZoomBuf ) );


	while ( 1 )
	{

		/* 读入摄像头数据 */
		iError = tVideoDevice.ptOPr->GetFrame ( &tVideoDevice,&tVideoBuf );
		if ( iError )
		{
			DBG_PRINTF ( "GetFrame for %s error!\n", argv[1] );
			return -1;
		}

		ptVideoBufCur = &tVideoBuf;
		/* 判断输入输出格式
		 * 如果两者不相同则需要进行转换
		*/
		if ( iPixelFormatOfVideo != iPixelFormatOfDisp )
		{
			iError = ptVideoConvert->Convert ( &tVideoBuf,&tConvertBuf );
            DBG_PRINTF("Convert %s, ret = %d\n", ptVideoConvert->name, iError);
			if ( iError )
			{
				DBG_PRINTF ( "Convert for %s error!\n", argv[1] );
				return -1;
			}

			ptVideoBufCur = &tConvertBuf;
		}

		//如果从video设备中读取到的图像分辨率大于LCD, 缩放
		if ( ( ptVideoBufCur->tPixelDatas.iHeight > iLcdHight ) || ( ptVideoBufCur->tPixelDatas.iWidth > iLcdWidth ) )
		{
            /* 确定缩放后的分辨率 */
            /* 把图片按比例缩放到VideoMem上, 居中显示
             * 1. 先算出缩放后的大小
             */
			k = ( float ) ptVideoBufCur->tPixelDatas.iHeight / ptVideoBufCur->tPixelDatas.iWidth; // 获取原先图片的长宽比

			tZoomBuf.tPixelDatas.iWidth  = iLcdWidth ;
			tZoomBuf.tPixelDatas.iHeight   = iLcdWidth * k ;

			//如果图片的高度大于LCD高度 限制图片高度为LCD高度 宽度按比例缩放
			if ( tZoomBuf.tPixelDatas.iHeight > iLcdHight )
			{
				tZoomBuf.tPixelDatas.iHeight = iLcdHight;
				tZoomBuf.tPixelDatas.iWidth = tZoomBuf.tPixelDatas.iHeight / k ;
			}

			tZoomBuf.tPixelDatas.iBpp	 = iLcdBpp;
			tZoomBuf.tPixelDatas.iLineBytes = tZoomBuf.tPixelDatas.iWidth * tZoomBuf.tPixelDatas.iBpp / 8;
			tZoomBuf.tPixelDatas.iTotalBytes = tZoomBuf.tPixelDatas.iLineBytes * tZoomBuf.tPixelDatas.iHeight;
			
			if ( !tZoomBuf.tPixelDatas.aucPixelDatas  )
			{
			    tZoomBuf.tPixelDatas.aucPixelDatas = malloc ( tZoomBuf.tPixelDatas.iTotalBytes );
				return -1;
			}
            
			//使用函数进行缩放
			PicZoom ( &ptVideoBufCur->tPixelDatas , &tZoomBuf.tPixelDatas );
			
			ptVideoBufCur = &tZoomBuf;
		}

		//算出图像起始位置
		iTopLeftX = (iLcdWidth -ptVideoBufCur->tPixelDatas.iWidth ) /2;
		iTopLeftY = (iLcdHight -ptVideoBufCur->tPixelDatas.iHeight ) /2;

		PicMerge(iTopLeftX,iTopLeftY,&ptVideoBufCur->tPixelDatas,&tFrameBuf.tPixelDatas);
		
		/* 把framebuffer的数据刷到LCD上, 显示 */
		FlushPixelDatasToDev(&tFrameBuf.tPixelDatas);

		iError=tVideoDevice.ptOPr->PutFrame(&tVideoDevice,&tVideoBuf);
		if ( iError )
		{
			DBG_PRINTF ( "PutFrame for %s error!\n", argv[1] );
			return -1;
		}

	}


	return 0;
}

