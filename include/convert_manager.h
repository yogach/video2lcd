
#ifndef __CONVERT_MANAGER__H__
#define __CONVERT_MANAGER__H__

#include <video_manager.h>

typedef struct VideoConvert
{
  char *name;
  int (*isSupport)(int iPixelFormatIn, int iPixelFormatOut); //�ж��Ƿ�֧�ִ˸�ʽ
  int (*Convert)(PT_VideoBuf ptVideoBufIn,PT_VideoBuf ptVideoBufOut);
  int (*ConvertExit)(PT_VideoBuf ptVideoBufOut);

  struct VideoConvert *ptNext;
}T_VideoConvert,*PT_VideoConvert;


int VideoConvertInit (void);
int Rgb2RgbConvertInit ( void );
int Yuv2RgbConvertInit ( void );


#endif //__CONVERT_MANAGER__H__
