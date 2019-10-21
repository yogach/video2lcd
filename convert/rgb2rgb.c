#include <config.h>
#include <convert_manager.h>


static int isSupportRgb2Rgb ( int iPixelFormatIn, int iPixelFormatOut )
{
	if ( iPixelFormatIn !=V4L2_PIX_FMT_RGB565 )
	{
		return 0;
	}

	if ( ( iPixelFormatOut!=V4L2_PIX_FMT_RGB565 ) && ( iPixelFormatOut!=V4L2_PIX_FMT_RGB32 ) )
	{
		return 0;
	}

	return 1;

}


static int Rgb2RgbConvert ( PT_VideoBuf ptVideoBufIn,PT_VideoBuf ptVideoBufOut )
{

	PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
	PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;

	int r,g,b,x,y;
	int color;

	unsigned int* pdwDest;
	unsigned short* pwSrc = ( unsigned short* ) ptPixelDatasIn->aucPixelDatas;


	if ( ptVideoBufIn->iPixelFormat !=V4L2_PIX_FMT_RGB565 )
	{
		DBG_PRINTF ( "can't support this PixelFormat" );
		return -1;
	}

	//设置分辨率
	ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
	ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth;

	//如果输出的格式也为RGB565 直接进行复制
	if ( ptVideoBufOut->iPixelFormat ==V4L2_PIX_FMT_RGB565 )
	{

		ptPixelDatasOut->iBpp = 16;
		ptPixelDatasOut->iLineBytes = ptPixelDatasOut->iWidth *ptPixelDatasOut->iBpp /8;
		ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;

		if ( !ptPixelDatasOut->aucPixelDatas )
		{

			ptPixelDatasOut->aucPixelDatas = malloc ( ptPixelDatasOut->iTotalBytes );
			if ( ptPixelDatasOut->aucPixelDatas ==NULL )
			{
				DBG_PRINTF ( "malloc error\r\n" );
				return -1;
			}
		}

		memcpy ( ptPixelDatasOut->aucPixelDatas, ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->iTotalBytes );
		return 0;

	}
	else if ( ptVideoBufOut->iPixelFormat ==V4L2_PIX_FMT_RGB32 )
	{
		ptPixelDatasOut->iBpp = 32;
		ptPixelDatasOut->iLineBytes = ptPixelDatasOut->iWidth *ptPixelDatasOut->iBpp /8;
		ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;

		if ( !ptPixelDatasOut->aucPixelDatas )
		{

			ptPixelDatasOut->aucPixelDatas = malloc ( ptPixelDatasOut->iTotalBytes );
			if ( ptPixelDatasOut->aucPixelDatas ==NULL )
			{
				DBG_PRINTF ( "malloc error\r\n" );
				return -1;
			}
		}

		//指针赋值
		pdwDest = ( unsigned int* ) ptPixelDatasOut->aucPixelDatas;

		for ( y=0; y<ptPixelDatasOut->iHeight; y++ )
		{
			for ( x=0; x<ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth; x++ )
			{
				color = *pwSrc++; // 先将*pwSrc赋值给color， 后指针自加
				/* 从RGB565格式的数据中提取出R,G,B */

				r = color >> 11;
				g = ( color >> 5 ) & 0x3f; //取其中低6位
				b = color & 0x1f;        //取其中低5位

				/* 把r,g,b转为0x00RRGGBB的32位数据 */
				color = ( ( r << 3 ) <<16 ) || ( ( g << 2 ) << 8 ) || ( b << 3 );

				*pdwDest = color;
				pdwDest++;

			}
		}

		return 0;
	}

	return -1;

}


static int Rgb2RgbConvertExit ( PT_VideoBuf ptVideoBufOut )
{
	if ( ptVideoBufOut->tPixelDatas.aucPixelDatas )
	{
		free ( ptVideoBufOut->tPixelDatas.aucPixelDatas );
		ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
	}
	return 0;

}


static T_VideoConvert g_tRgb2RgbConvert =
{
	.name = "rgb2rgb",
	.isSupport =isSupportRgb2Rgb,
	.Convert  = Rgb2RgbConvert,
	.ConvertExit =Rgb2RgbConvertExit,
};


int Rgb2RgbConvertInit ( void )
{
	return  RegisterVideoConvertOpr ( &g_tRgb2RgbConvert );
}

