#include <config.h>
#include <convert_manager.h>


static int isSupportRgb2Rgb ( int iPixelFormatIn, int iPixelFormatOut )
{

}
static int Rgb2RgbConvert ( PT_VideoBuf ptVideoBufIn,PT_VideoBuf ptVideoBufOut )
{

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


static T_VideoConvert g_Rgb2RgbConvert =
{
	.name = "Rgb2Rgb",
	.isSupport =isSupportRgb2Rgb,
	.Convert  = Rgb2RgbConvert,
	.ConvertExit =Rgb2RgbConvertExit,
};


int Rgb2RgbConvertInit ( void )
{
	return  RegisterVideoConvertOpr ( &g_Rgb2RgbConvert );
}

