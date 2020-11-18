#pragma once

#ifndef _CANONCAMERA_H_
#define _CANONCAMEARA_H_

#include <Windows.h>  
#include<process.h>
#include<vector>

#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "EDSDKErrors.h"

#include "CanonDebug.h"

//#include "CameraModel.h"
//#include "CameraModelLegacy.h"
//#include "CameraController.h"
//#include "CameraEventListener.h"

class PhotoHandler
{
public:

	/*virtual void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error) = 0;
	virtual void photoDownloaded(const std::string & downloadPath, EdsError error) = 0;
	virtual std::string photoDownloadDirectory() = 0;*/

	void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error);
	void photoDownloaded(const std::string & downloadPath, EdsError error);
	std::string photoDownloadDirectory();
};


class mycanons
{
public:
	mycanons();
	~mycanons();

	EdsError setup();
	//void takePictureRef(PhotoHandler * photoHandler);
	void takePictureRef();
	//void downloadImageRef(EdsDirectoryItemRef dirItem, PhotoHandler * photoHandler);
	void downloadImageRef(EdsDirectoryItemRef dirItem);
	bool sendCommand(EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam);
	void startLiveView();
	void endLiveView();

	EdsError downloadImage(EdsDirectoryItemRef directoryItem, int positionTem);
	EdsError takePicture(EdsCameraRef* cameratem);
	EdsError detectCameras();
	EdsError getCamera(EdsCameraRef * camera, EdsInt32 cameraindex);
	EdsError takePicturesFromCams();

	void takePicturesByMultiThread();
	void listenKeyEvent();

	static UINT WINAPI takePictureThreadL(void* lParam);
	static UINT WINAPI takePictureThreadM(void* lParam);
	static UINT WINAPI takePictureThreadR(void* lParam);

	EdsError getVolume(EdsCameraRef camera, EdsVolumeRef *volume);
	EdsError getDCIMFolder(EdsVolumeRef volume, EdsDirectoryItemRef * directoryItem, int positionTem);
	//EdsError getNewImageItem(EdsVolumeRef volume, EdsDirectoryItemRef * directoryItem);

	EdsError performAction();
	EdsError shutdown();
	EdsError getDeviceInfo(EdsCameraRef cam);

	/** ���������߳�*/
	bool OpenListenThread();
	/** �����߳�*/
	static UINT WINAPI ListenThread(void* pParam);
	/** �ر���������߳�*/
	bool CloseListenTread();

protected:

	bool                bCameraIsConnected;
	bool                bIsLiveView;
	bool                bFrameNew;

	static EdsError EDSCALLBACK handleObjectEvent(
		EdsUInt32 inEvent, 
		EdsBaseRef inRef, 
		EdsVoid* inContext
	);

	static EdsError EDSCALLBACK handlePropertyEvent(
		EdsUInt32 inEvent, 
		EdsUInt32 inPropertyID, 
		EdsUInt32 inParam, 
		EdsVoid* inContext
	);
	static EdsError EDSCALLBACK handleStateEvent(
		EdsUInt32 inEvent, 
		EdsUInt32 inParam, 
		EdsVoid* inContext
	);
	
private:
	//EdsUInt32	 count = 0;
	bool isSDKLoaded = false;
	//EdsCameraListRef cameraList = NULL;
	EdsCameraRef camera = NULL;
	EdsCameraRef* cameraSet;
	EdsDeviceInfo* cameraInfoSet;
	EdsUInt32	camNum = 0;
	EdsUInt32	busyCamIndex = 0;
	const std::vector<std::string> camSetName = { "left","middle","right" };
	EdsUInt32 pushButtonNum = 0;
	//���������
	HANDLE hMutexL;
	HANDLE hMutexM;
	HANDLE hMutexR;
	//HANDLE *hMutex;
	//�߳̾��
	/*HANDLE hThreadL;
	HANDLE hThreadM;
	HANDLE hThreadR;*/
	

	EdsDeviceInfo deviceInfo;
	EdsInt32 deviceId;

	/** ����߳̾�� */
	HANDLE  m_hComm;
	/** �߳��˳���־���� */
	static bool s_bExit;
	/** �߳̾�� */
	volatile HANDLE   hThread;
};



#endif // ! _CANONCAMERA_H_
