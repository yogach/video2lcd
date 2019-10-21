
#ifndef __CONVERT_MANAGER__H__
#define __CONVERT_MANAGER__H__


#include <config.h>
#include <video_manager.h>
#include <linux/videodev2.h>


typedef struct VideoConvert
{
  char *name;
  int (*isSupport)(int iPixelFormatIn, int iPixelFormatOut); //判断是否支持此格式
  int (*Convert)(PT_VideoBuf ptVideoBufIn,PT_VideoBuf ptVideoBufOut);
  int (*ConvertExit)(PT_VideoBuf ptVideoBufOut);
  struct VideoConvert *ptNext;
}T_VideoConvert,*PT_VideoConvert;


int RegisterVideoConvertOpr (PT_VideoConvert ptVideoConvertOpr);
void ShowVideoConvertOpr (void);
PT_VideoConvert GetVideoConvertOpr (char * pcName);



int VideoConvertInit (void);
int Rgb2RgbConvertInit ( void );
int Yuv2RgbConvertInit ( void );
int Mjpeg2RgbConvertInit ( void );


#endif //__CONVERT_MANAGER__H__
