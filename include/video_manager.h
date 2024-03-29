#ifndef __VIDEO_MANAGER_H__
#define __VIDEO_MANAGER_H__

#include <pic_operation.h>
#include <linux/videodev2.h>


#define NB_BUFFER 4

struct VideoDevice;
struct VideoOpr;
typedef struct VideoDevice T_VideoDevice,*PT_VideoDevice;
typedef struct VideoOpr T_VideoOpr,*PT_VideoOpr;

typedef struct VideoBuf
{
	T_PixelDatas tPixelDatas;
	int iPixelFormat;           //像素格式
} T_VideoBuf,*PT_VideoBuf;


//video设备属性以及操作函数
struct VideoDevice
{
	int iFd; //文件句柄
	int iPixelFormat;           //像素格式
	int iWidth;   //宽度
	int iHeight; //高度
	int iVideoBufCnt; //一共有几个videobuff
	int iVideoBufMaxLen; //videobuff的最大长度
	int iVideoBufCurIndex;//使用的是第几个videobuff
    unsigned int iVideoCapabilities;//设备支持的操作模式，常见的值有 V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING

	unsigned char* pucVideBuf[NB_BUFFER]; //用于存储videobuff的起始地址

	PT_VideoOpr ptOPr;

};

//video设备操作函数
struct VideoOpr
{
	char* name;
	int ( *InitDevice ) ( char* strName, PT_VideoDevice ptVideoDevice ); // 初始化设备
	int ( *ExitDevice ) ( PT_VideoDevice ptVideoDevice );           //退出设备
	int ( *GetFrame ) ( PT_VideoDevice ptVideoDevice,PT_VideoBuf ptVideoBuf ); //取得数据
	int ( *PutFrame ) ( PT_VideoDevice ptVideoDevice,PT_VideoBuf ptVideoBuf ); //释放数据
	int ( *StartDevice ) ( PT_VideoDevice ptVideoDevice ); //启动设备
	int ( *StopDevice ) ( PT_VideoDevice ptVideoDevice ); //停止设备
	int ( *GetFormat ) ( PT_VideoDevice ptVideoDevice );

	struct VideoOpr* ptNext;

};


int RegisterVideoOpr ( PT_VideoOpr ptVideoOpr );
void ShowVideoOpr ( void );
PT_VideoOpr GetVideoOpr ( char* pcName );
int VideoInit ( void );

int V4l2Init ( void );
int	 VideoDeviceInit(char *DeviceName, PT_VideoDevice ptVideoDevice);


#endif
