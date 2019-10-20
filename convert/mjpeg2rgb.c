#include <config.h>
#include <convert_manager.h>


static int isSupportMjpeg2Rgb ( int iPixelFormatIn, int iPixelFormatOut )
{

}
static int Mjpeg2RgbConvert ( PT_VideoBuf ptVideoBufIn,PT_VideoBuf ptVideoBufOut )
{

}
static int Mjpeg2RgbConvertExit ( PT_VideoBuf ptVideoBufOut )
{
	if ( ptVideoBufOut->tPixelDatas.aucPixelDatas )
	{
		free ( ptVideoBufOut->tPixelDatas.aucPixelDatas );
		ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
	}
	return 0;

}


static T_VideoConvert g_Mjpeg2RgbConvert =
{
	.name = "Rgb2Rgb",
	.isSupport =isSupportMjpeg2Rgb,
	.Convert  = Mjpeg2RgbConvert,
	.ConvertExit =Mjpeg2RgbConvertExit,
};


int Mjpeg2RgbConvertInit ( void )
{
	return  RegisterVideoConvertOpr ( &g_Mjpeg2RgbConvert );
}


