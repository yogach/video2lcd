#ifdef  __VIDEO_MANAGER_H__
#define __VIDEO_MANAGER_H__

#include <pic_operation.h>


#define NB_BUFFER 4 

struct VideoDevice;
struct VideoOpr; 
typedef struct VideoDevice T_VideoDevice ,*PT_VideoDevice;
typedef struct VideoOpr T_VideoOpr ,*PT_VideoOpr;

typedef struct VideoBuf
{
   T_PixelDatas tPixelDatas;
   int iPixelFormat;           //���ظ�ʽ
}T_VideoBuf,*PT_VideoBuf;


//video�豸�����Լ�����
struct VideoDevice
{
   int iFd; //�ļ����
   int iPixelFormat;           //���ظ�ʽ
   int iWidth;   //���
    int iHeight; //�߶�
       int iVideoBufCnt; //һ���м���videobuff
    int iVideoBufMaxLen; //videobuff����󳤶�
    int iVideoBufCurIndex;//ʹ�õ��ǵڼ���videobuff

    unsigned char *pucVideBuf[NB_BUFFER]; //���ڴ洢videobuff����ʼ��ַ

	PT_VideoOpr ptVideoOpr;

};

//video�豸��������
struct VideoOpr 
{
  char *name;
  int (*InitDevice)(char* strName, PT_VideoDevice ptVideoDevice); // ��ʼ���豸
  int (*ExitDevice)(PT_VideoDevice ptVideoDevice);                //�˳��豸
  int (*GetFrame)(PT_VideoDevice ptVideoDevice,PT_VideoBuf ptVideoBuf); //ȡ������
  int (*PutFrame)(PT_VideoDevice ptVideoDevice,PT_VideoBuf ptVideoBuf); //�ͷ�����
  int (*StartDevice)(PT_VideoDevice ptVideoDevice);//�����豸
  int (*StopDevice)(PT_VideoDevice ptVideoDevice);//ֹͣ�豸
  int (*GetFormat)(PT_VideoDevice ptVideoDevice);

  struct VideoOpr *ptNext;

};


int RegisterVideoOpr (PT_VideoOpr ptVideoOpr);
void ShowVideoOpr (void);
PT_VideoOpr GetVideoOpr (char * pcName);
int VideoInit (void);



#endif
