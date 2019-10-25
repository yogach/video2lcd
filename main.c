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

	T_VideoBuf tVideoBuf; //���ڴ洢videoԭʼ�豸
	T_VideoBuf tConvertBuf; //���ڴ洢ת�����video����
	T_VideoBuf tZoomBuf;//���ڴ洢���ź��video����
	PT_VideoBuf ptVideoBufCur;
    float k;

    int iTopLeftX,iTopLeftY;
	

	if ( argc != 2 )
	{
		DBG_PRINTF ( "Usage:\n" );
		DBG_PRINTF ( "%s </dev/video0,1,...>\n", argv[0] );
		return 0;
	}

	/* ע����ʾ�豸 */
	DisplayInit();

	SelectAndInitDefaultDispDev ( "fb" );
	GetDispResolution ( &iLcdWidth, &iLcdHight, &iLcdBpp );
	//��ȡlcd��Ϣ
	GetVideoBufForDisplay ( &tFrameBuf );
	iPixelFormatOfDisp = tFrameBuf.iPixelFormat;

	//��ʼ��video�豸
	VideoInit();

	//��ʼ��������ָ����video�豸 tVideoDevice�ڰ���video�豸�����Լ���������
	iError = VideoDeviceInit ( argv[1], &tVideoDevice );
	if ( iError != 0 )
	{
		DBG_PRINTF ( "VideoDeviceInit error\r\n" );
		return -1;
	}

	iPixelFormatOfVideo = tVideoDevice.iPixelFormat;

	//video��ʽת����ʼ��
	VideoConvertInit();

	//����ܹ�֧�ֵ�������� ���ظ�ʽת������
	ptVideoConvert =  GetVideoConvertForFormats ( iPixelFormatOfVideo, iPixelFormatOfDisp );
	if ( ptVideoConvert == NULL )
	{

		DBG_PRINTF ( "can't support this Formats convert \r\n" );
		return -1;

	}


	/* ��������ͷ�豸 */
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

		/* ��������ͷ���� */
		iError = tVideoDevice.ptOPr->GetFrame ( &tVideoDevice,&tVideoBuf );
		if ( iError )
		{
			DBG_PRINTF ( "GetFrame for %s error!\n", argv[1] );
			return -1;
		}

		ptVideoBufCur = &tVideoBuf;
		/* �ж����������ʽ
		 * ������߲���ͬ����Ҫ����ת��
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

		//�����video�豸�ж�ȡ����ͼ��ֱ��ʴ���LCD, ����
		if ( ( ptVideoBufCur->tPixelDatas.iHeight > iLcdHight ) || ( ptVideoBufCur->tPixelDatas.iWidth > iLcdWidth ) )
		{
            /* ȷ�����ź�ķֱ��� */
            /* ��ͼƬ���������ŵ�VideoMem��, ������ʾ
             * 1. ��������ź�Ĵ�С
             */
			k = ( float ) ptVideoBufCur->tPixelDatas.iHeight / ptVideoBufCur->tPixelDatas.iWidth; // ��ȡԭ��ͼƬ�ĳ����

			tZoomBuf.tPixelDatas.iWidth  = iLcdWidth ;
			tZoomBuf.tPixelDatas.iHeight   = iLcdWidth * k ;

			//���ͼƬ�ĸ߶ȴ���LCD�߶� ����ͼƬ�߶�ΪLCD�߶� ��Ȱ���������
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
            
			//ʹ�ú�����������
			PicZoom ( &ptVideoBufCur->tPixelDatas , &tZoomBuf.tPixelDatas );
			
			ptVideoBufCur = &tZoomBuf;
		}

		//���ͼ����ʼλ��
		iTopLeftX = (iLcdWidth -ptVideoBufCur->tPixelDatas.iWidth ) /2;
		iTopLeftY = (iLcdHight -ptVideoBufCur->tPixelDatas.iHeight ) /2;

		PicMerge(iTopLeftX,iTopLeftY,&ptVideoBufCur->tPixelDatas,&tFrameBuf.tPixelDatas);
		
		/* ��framebuffer������ˢ��LCD��, ��ʾ */
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

