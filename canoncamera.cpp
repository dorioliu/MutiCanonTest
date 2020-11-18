#include "canoncamera.h"
#include <iostream>

EdsError EDSCALLBACK DownloadImageProgress(EdsUInt32	inPercent
	, EdsVoid*	inContext
	, EdsBool*	outCancel)
{
	printf("Canon: downloading image: %d.\n", (int)inPercent);
	return EDS_ERR_OK;
};
//CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo);


static PhotoHandler * SingletonPhotoHandler = NULL;

/** 线程退出标志 */
bool mycanons::s_bExit = false;

mycanons::mycanons() : hThread(INVALID_HANDLE_VALUE),
bCameraIsConnected(false), bIsLiveView(false)
{
	setup();
}

EdsError mycanons::setup()
{
	EdsError errtem = EDS_ERR_OK;

	/*Initialization of SDK, When using the EDSDK libraries,
	you must call this API once before using EDSDK APIs*/

	errtem = EdsInitializeSDK();
	if (errtem == EDS_ERR_OK)
	{
		isSDKLoaded = true;
	}

	errtem = detectCameras();

	hMutexL = CreateMutex(NULL, FALSE, NULL);
	hMutexM = CreateMutex(NULL, FALSE, NULL);
	hMutexR = CreateMutex(NULL, FALSE, NULL);

	//hMutex = new HANDLE[camNum];
	/*for (int i = 0; i < camNum; i++)
	{
		hMutex[i] = CreateMutex(NULL, FALSE, NULL);
		WaitForSingleObject(hMutex[i], INFINITE);
	}*/

	WaitForSingleObject(hMutexL, INFINITE);
	WaitForSingleObject(hMutexM, INFINITE);
	WaitForSingleObject(hMutexR, INFINITE);

	///** 打开会话*/
	//errtem = EdsOpenSession(camera);
	//if (errtem != EDS_ERR_OK)
	//{
	//	std::cout << "Couldn't open the session with camera id: " << deviceId << std::endl;
	//	return errtem;
	//	exit(-1);
	//}

	////Set Object Event Handler
	//if (errtem == EDS_ERR_OK)
	//{
	//	errtem = EdsSetObjectEventHandler(camera, kEdsObjectEvent_All, mycanons::handleObjectEvent, (EdsVoid*)this);
	//}

	////Set Property Event Handler
	//if (errtem == EDS_ERR_OK)
	//{
	//	errtem = EdsSetPropertyEventHandler(camera, kEdsPropertyEvent_All, mycanons::handlePropertyEvent, (EdsVoid*)this);
	//}

	////Set State Event Handler
	//if (errtem == EDS_ERR_OK)
	//{
	//	errtem = EdsSetCameraStateEventHandler(camera, kEdsStateEvent_All, mycanons::handleStateEvent, (EdsVoid*)this);
	//}


	bCameraIsConnected = true;

	return errtem;
}

//void mycanons::takePictureRef(PhotoHandler * photoHandler)
//{
//	if (!bCameraIsConnected)
//		return;
//
//	SingletonPhotoHandler = photoHandler;
//
//	std::cout <<"Attempting to take picture" << std::endl;
//
//	sendCommand(camera, kEdsCameraStatusCommand_UILock, 0);
//	sendCommand(camera, kEdsCameraCommand_TakePicture, 0);
//
//}

void mycanons::takePictureRef()
{

	if (!bCameraIsConnected)
		return;

	std::cout << "Attempting to take picture" << std::endl;

	sendCommand(camera, kEdsCameraStatusCommand_UILock, 0);
	sendCommand(camera, kEdsCameraCommand_TakePicture, 0);

}

//void mycanons::downloadImageRef(EdsDirectoryItemRef dirItem, PhotoHandler * photoHandler)
//{
void mycanons::downloadImageRef(EdsDirectoryItemRef dirItem)
{
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;

	// Get info about new image.
	EdsDirectoryItemInfo dir_item_info;
	err = EdsGetDirectoryItemInfo(dirItem, &dir_item_info);
	if (err != EDS_ERR_OK) {
		printf("Canon: error while trying to get more info about image to be downloaded.\n");
	}

	// Created file stream to download image.
	/*std::string downloadDest = "/tmp/canon";
	if (photoHandler)
	{
	downloadDest = photoHandler->photoDownloadDirectory();
	}
	*/
	std::string downloadPath = "./";

	if (err == EDS_ERR_OK) {
		err = EdsCreateFileStream(downloadPath.c_str(),
			kEdsFileCreateDisposition_CreateAlways,
			kEdsAccess_ReadWrite,
			&stream);
	}
	if (err != EDS_ERR_OK) {
		printf("Canon: error while creating download stream.\n");
	}

	// Set progress
	if (err == EDS_ERR_OK) {
		err = EdsSetProgressCallback(stream, DownloadImageProgress, kEdsProgressOption_Periodically, this);
	}
	if (err != EDS_ERR_OK) {
		printf("Canon: error while setting download progress function.\n");
	}

	// Download image.
	if (err == EDS_ERR_OK) {
		err = EdsDownload(dirItem, dir_item_info.size, stream);
	}
	if (err != EDS_ERR_OK) {
		printf("Canon: error while downloading item.\n");
	}

	// Tell we're ready.
	if (err == EDS_ERR_OK) {
		std::cout << "Downloaded image to " << downloadPath.c_str() << "\n" << std::endl;
		err = EdsDownloadComplete(dirItem);
	}
	if (err != EDS_ERR_OK) {
		printf("Canon: error while telling we're ready with the file download.\n");
	}

	// Release dir item.
	/*
	if(dirItem != NULL) {
	err = EdsRelease(dirItem);
	if(err != EDS_ERR_OK) {
	printf("Canon: error releasing dir item when downloading.\n");
	}
	dirItem = NULL;
	}
	*/

	// Release stream
	if (stream != NULL) {
		err = EdsRelease(stream);
		if (err != EDS_ERR_OK) {
			printf("Canon: error while releasing download stream.\n");
		}
		stream = NULL;
	}

	/*if (err != EDS_ERR_OK)
	{
	downloadDest = "";
	}*/

	/*if (photoHandler)
	{
	photoHandler->photoDownloaded(downloadPath, err);
	}*/
}

bool mycanons::sendCommand(EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam)
{
	EdsError err = EDS_ERR_OK;

	err = EdsSendCommand(inCameraRef, inCommand, inParam);

	if (err != EDS_ERR_OK) {
		std::cout << "error while sending command " << CanonErrorToString(err) << "." << std::endl;
		if (err == EDS_ERR_DEVICE_BUSY) {
			return false;
		}
	}

	return true;
}

void mycanons::startLiveView()
{
	std::cout << "Cinder-Canon :: start live view" << std::endl;
	EdsError err = EDS_ERR_OK;

	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);

	// PC live view starts by setting the PC as the output device for the live view image.
	if (err == EDS_ERR_OK)
	{
		device |= kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);
		bIsLiveView = true;
	}
}

void mycanons::endLiveView()
{
	EdsError err = EDS_ERR_OK;

	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);

	// PC live view ends if the PC is disconnected from the live view image output device.
	if (err == EDS_ERR_OK)
	{
		device &= ~kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);
	}

	bIsLiveView = false;
}

EdsError mycanons::getCamera(EdsCameraRef *camera, EdsInt32 cameraindex)
{
	EdsError errtem = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	EdsUInt32 count = 0;

	// Get camera list
	errtem = EdsGetCameraList(&cameraList);
	// Get number of cameras
	if (errtem == EDS_ERR_OK)
	{
		errtem = EdsGetChildCount(cameraList, &count);
		if (count == 0)
		{
			errtem = EDS_ERR_DEVICE_NOT_FOUND;
		}
		else
		{
			if (cameraindex>static_cast<EdsUInt32>(count - 1))
			{
				std::cout << " only " << count << "camera(s) are detected. " << "There is no camera index " << cameraindex << std::endl;
				errtem = EDS_ERR_DEVICE_NOT_FOUND;
			}
		}
	}

	// Get camera retrieved
	if (errtem == EDS_ERR_OK)
	{
		// cameraindex = 0, means the first camera
		errtem = EdsGetChildAtIndex(cameraList, cameraindex, camera);
	}

	// Release camera list
	if (cameraList != NULL)
	{
		EdsRelease(cameraList);
		cameraList = NULL;
	}
	return errtem;

}


EdsError mycanons::takePicturesFromCams()
{
	int keystate = 0;
	keystate = GetKeyState(VK_SPACE);
	if (keystate < 0) // space 被按下
	{
		if (camNum > 0)
		{
			for (EdsUInt32 j = 0; j < camNum; j++)
			{
				EdsOpenSession(cameraSet[j]);
			}

			for (EdsUInt32 j = 0; j<camNum; j++)
			{
				busyCamIndex = j;
				/** 打开会话*/
				//EdsOpenSession(cameraSet[j]);
				//Sleep(10);

				takePicture(&cameraSet[j]);
				Sleep(800);

				EdsVolumeRef cameraVol;
				getVolume(cameraSet[j], &cameraVol);

				EdsDirectoryItemRef camearaDirItem;
				getDCIMFolder(cameraVol, &camearaDirItem, busyCamIndex);

				if (cameraVol)
				{
					EdsRelease(cameraVol);
				}
				if (camearaDirItem)
				{
					EdsRelease(camearaDirItem);
				}

				// 重新建立联系
				//EdsCloseSession(cameraSet[j]);
				//Sleep(10);
			}

			for (EdsUInt32 j = 0; j < camNum; j++)
			{
				// 重新建立联系
				EdsCloseSession(cameraSet[j]);
				Sleep(10);
			}

		}
		
		do {
			keystate = GetKeyState(VK_SPACE);
			if (keystate < 0)
				continue;
			else
				break;
		} while (true);

		pushButtonNum++;

	}

	return EDS_ERR_OK;
}

void mycanons::takePicturesByMultiThread()
{
	
	if (camNum == 3)
	{
		/** 线程ID */
		UINT threadIdL;
		HANDLE hThreadL= (HANDLE)_beginthreadex(NULL, 0,takePictureThreadL, this, 0, &threadIdL);
		UINT threadIdM;
		HANDLE hThreadM = (HANDLE)_beginthreadex(NULL, 0, takePictureThreadM, this, 0, &threadIdM);
		UINT threadIdR;
		HANDLE hThreadR = (HANDLE)_beginthreadex(NULL, 0, takePictureThreadR, this, 0, &threadIdR);

		//CloseHandle(hThreadL);
		//CloseHandle(hThreadM);
		//CloseHandle(hThreadR);

		/*WaitForSingleObject(hMutexL, INFINITE);
		WaitForSingleObject(hMutexM, INFINITE);
		WaitForSingleObject(hMutexR, INFINITE);
*/

		/*::WaitForSingleObject(hThreadL, INFINITE);
		::WaitForSingleObject(hThreadM, INFINITE);
		::WaitForSingleObject(hThreadR, INFINITE);

		CloseHandle(hThreadL);
		CloseHandle(hThreadM);
		CloseHandle(hThreadR);*/

	}
	else if (camNum == 2)
	{

		/** 线程ID */
		UINT threadIdL;
		HANDLE hThreadL = (HANDLE)_beginthreadex(NULL, 0, takePictureThreadL, this, 0, &threadIdL);
		UINT threadIdR;
		HANDLE hThreadR = (HANDLE)_beginthreadex(NULL, 0, takePictureThreadR, this, 0, &threadIdR);

		::WaitForSingleObject(hThreadL, INFINITE);
		::WaitForSingleObject(hThreadR, INFINITE);

		CloseHandle(hThreadL);
		CloseHandle(hThreadR);



	}
	else if (camNum == 1)
	{
		UINT threadIdL;
		HANDLE hThreadL = (HANDLE)_beginthreadex(NULL, 0, takePictureThreadL, this, 0, &threadIdL);

		::WaitForSingleObject(hThreadL, INFINITE);

		CloseHandle(hThreadL);
	}

}

void mycanons::listenKeyEvent()
{
	int keystate = 0;
	keystate = GetKeyState(VK_SPACE);
	if (keystate < 0) // space 被按下
	{

		//takePicturesByMultiThread();
		Sleep(10);

		do {
			keystate = GetKeyState(VK_SPACE);
			if (keystate < 0)
				continue;
			else
				break;
		} while (true);

		ReleaseMutex(hMutexL);
		ReleaseMutex(hMutexM);
		ReleaseMutex(hMutexR);

	/*	ReleaseMutex(hMutex[0]);
		ReleaseMutex(hMutex[1]);
		ReleaseMutex(hMutex[2]);*/

		Sleep(5);
	
	    WaitForSingleObject(hMutexL, INFINITE);
		WaitForSingleObject(hMutexM, INFINITE);
		WaitForSingleObject(hMutexR, INFINITE);

		/*WaitForSingleObject(hMutex[0], INFINITE);
		WaitForSingleObject(hMutex[1], INFINITE);
		WaitForSingleObject(hMutex[2], INFINITE);
*/
		//WaitForMultipleObjects(camNum, hMutex, true,INFINITE);

		pushButtonNum++;

	}

}

UINT WINAPI mycanons::takePictureThreadL(void * lParam)
{
	/** 得到本类的指针 */
	mycanons *pCamera = reinterpret_cast<mycanons*>(lParam);


	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	EdsOpenSession(pCamera->cameraSet[0]);

	while(1)
	{
		
		WaitForSingleObject(pCamera->hMutexL,INFINITE);
		
		pCamera->takePicture(pCamera->cameraSet);
		Sleep(1000);

		EdsVolumeRef cameraVol;
		pCamera->getVolume(pCamera->cameraSet[0], &cameraVol);

		EdsDirectoryItemRef camearaDirItem;
		pCamera->getDCIMFolder(cameraVol, &camearaDirItem,0);

		if (cameraVol)
		{
			EdsRelease(cameraVol);
		}
		if (camearaDirItem)
		{
			EdsRelease(camearaDirItem);
		}

		ReleaseMutex(pCamera->hMutexL);

		Sleep(5);
	}

	/*pCamera->takePicture(pCamera->cameraSet);
	Sleep(1000);

	EdsVolumeRef cameraVol;
	pCamera->getVolume(pCamera->cameraSet[0], &cameraVol);

	EdsDirectoryItemRef camearaDirItem;
	pCamera->getDCIMFolder(cameraVol, &camearaDirItem, 0);

	if (cameraVol)
	{
		EdsRelease(cameraVol);
	}
	if (camearaDirItem)
	{
		EdsRelease(camearaDirItem);
	}*/

 	EdsCloseSession(pCamera->cameraSet[0]);

	//Sleep(5);
	
	CoUninitialize();
	_endthread();
	return 0;

}

UINT WINAPI mycanons::takePictureThreadM(void * lParam)
{
	/** 得到本类的指针 */
	mycanons *pCamera = reinterpret_cast<mycanons*>(lParam);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	EdsOpenSession(pCamera->cameraSet[1]);

	while (1)
	{
		WaitForSingleObject(pCamera->hMutexM, INFINITE);

		pCamera->takePicture(pCamera->cameraSet+1);
		Sleep(1000);

		EdsVolumeRef cameraVol;
		pCamera->getVolume(pCamera->cameraSet[1], &cameraVol);

		EdsDirectoryItemRef camearaDirItem;
		pCamera->getDCIMFolder(cameraVol, &camearaDirItem,1);

		if (cameraVol)
		{
			EdsRelease(cameraVol);
		}
		if (camearaDirItem)
		{
			EdsRelease(camearaDirItem);
		}

		ReleaseMutex(pCamera->hMutexM);

		Sleep(5);
	}

	/*pCamera->takePicture(pCamera->cameraSet + 1);
	Sleep(1000);

	EdsVolumeRef cameraVol;
	pCamera->getVolume(pCamera->cameraSet[1], &cameraVol);

	EdsDirectoryItemRef camearaDirItem;
	pCamera->getDCIMFolder(cameraVol, &camearaDirItem, 1);

	if (cameraVol)
	{
		EdsRelease(cameraVol);
	}
	if (camearaDirItem)
	{
		EdsRelease(camearaDirItem);
	}*/
	
	

	EdsCloseSession(pCamera->cameraSet[1]);

	CoUninitialize();
	_endthread();
	return 0;

}

UINT WINAPI mycanons::takePictureThreadR(void * lParam)
{
	/** 得到本类的指针 */
	mycanons *pCamera = reinterpret_cast<mycanons*>(lParam);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	EdsOpenSession(pCamera->cameraSet[2]);

	while (1)
	{
		WaitForSingleObject(pCamera->hMutexR, INFINITE);

		pCamera->takePicture(pCamera->cameraSet + 2);
		Sleep(1000);

		EdsVolumeRef cameraVol;
		pCamera->getVolume(pCamera->cameraSet[2], &cameraVol);

		EdsDirectoryItemRef camearaDirItem;
		pCamera->getDCIMFolder(cameraVol, &camearaDirItem, 2);

		if (cameraVol)
		{
			EdsRelease(cameraVol);
		}
		if (camearaDirItem)
		{
			EdsRelease(camearaDirItem);
		}

		ReleaseMutex(pCamera->hMutexR);

		Sleep(5);
	}


	/*pCamera->takePicture(pCamera->cameraSet + 2);
	Sleep(1000);

	EdsVolumeRef cameraVol;
	pCamera->getVolume(pCamera->cameraSet[2], &cameraVol);

	EdsDirectoryItemRef camearaDirItem;
	pCamera->getDCIMFolder(cameraVol, &camearaDirItem, 2);

	if (cameraVol)
	{
		EdsRelease(cameraVol);
	}
	if (camearaDirItem)
	{
		EdsRelease(camearaDirItem);
	}*/

	EdsCloseSession(pCamera->cameraSet[2]);


	CoUninitialize();
	_endthread();

	return 0;
}


EdsError mycanons::getVolume(EdsCameraRef camera, EdsVolumeRef * volume)
{
	EdsError errtem = EDS_ERR_OK;
	EdsUInt32 counttem = 0;

	// Get the number of camera volumes
	errtem = EdsGetChildCount(camera, &counttem);
	if (errtem == EDS_ERR_OK && counttem == 0)
	{
		errtem = EDS_ERR_DIR_NOT_FOUND;
	}
	// Get initial volume
	if (errtem == EDS_ERR_OK)
	{
		errtem = EdsGetChildAtIndex(camera, 0, volume);
	}
	return errtem;
}

EdsError mycanons::getDCIMFolder(EdsVolumeRef volume, EdsDirectoryItemRef * directoryItem, int positionTem)
{
	EdsError errtem = EDS_ERR_OK;
	EdsDirectoryItemRef dirItem = NULL;
	EdsDirectoryItemInfo dirItemInfo;
	EdsUInt32 count = 0;
	// Get number of items under the volume
	errtem = EdsGetChildCount(volume, &count);
	if (errtem == EDS_ERR_OK && count == 0)
	{
		errtem = EDS_ERR_DIR_NOT_FOUND;
	}
	// Get DCIM folder
	for (int i = 0; i < count && errtem == EDS_ERR_OK; i++)
	{
		// Get the ith item under the specified volume
		if (errtem == EDS_ERR_OK)
		{
			errtem = EdsGetChildAtIndex(volume, i, &dirItem);
		}
		// Get retrieved item information
		if (errtem == EDS_ERR_OK)
		{
			errtem = EdsGetDirectoryItemInfo(dirItem, &dirItemInfo);
		}
		// Indicates whether or not the retrieved item is a DCIM folder.
		if (errtem == EDS_ERR_OK)
		{
			if (_stricmp(dirItemInfo.szFileName, "DCIM") == 0 &&
				dirItemInfo.isFolder == true)
			{
				directoryItem = &dirItem; // 返回的是DICM的地址
				EdsDirectoryItemRef newfileItem = NULL;

				{
					EdsUInt32 countem = 0;
					errtem = EdsGetChildCount(*directoryItem, &countem);
					if (errtem == EDS_ERR_OK && countem == 0)
					{
						errtem = EDS_ERR_DIR_NOT_FOUND;
					}

					EdsDirectoryItemRef dirItemtem = NULL;
					if (errtem == EDS_ERR_OK)
					{
						//返回的是DICM的地址
						errtem = EdsGetChildAtIndex(*directoryItem, 0, &dirItemtem);
					}

					EdsDirectoryItemInfo dirItemInfotem;
					// Get retrieved item information
					if (errtem == EDS_ERR_OK)
					{
						errtem = EdsGetDirectoryItemInfo(dirItemtem, &dirItemInfotem);
					}


					EdsUInt32 countem1 = 0;
					errtem = EdsGetChildCount(dirItemtem, &countem1);

					if (errtem == EDS_ERR_OK)
					{
						if (countem1>1)
							errtem = EdsGetChildAtIndex(dirItemtem, countem1 - 1, &newfileItem);
						else
							errtem = EdsGetChildAtIndex(dirItemtem, 0, &newfileItem);
					}

					EdsDirectoryItemInfo dirItemInfotem1;
					// Get retrieved item information
					if (errtem == EDS_ERR_OK)
					{
						errtem = EdsGetDirectoryItemInfo(newfileItem, &dirItemInfotem1);
					}
				}

				directoryItem = &newfileItem; // 返回的是最新一张图片的地址

											  //downloadImage(newfileItem);
				downloadImage(newfileItem, positionTem);

				break;
			}
		}

		// Release retrieved item
		if (dirItem)
		{
			EdsRelease(dirItem);
			dirItem = NULL;
		}
	}
	return errtem;
}

EdsError mycanons::getDeviceInfo(EdsCameraRef cam)
{
	EdsError errtem = EDS_ERR_OK;

	EdsDeviceInfo info;
	errtem = EdsGetDeviceInfo(cam, &info);
	if (errtem != EDS_ERR_OK)
	{
		std::cout << " Couldn't retrieve camera info" << std::endl;
		return errtem;
	}
	// print camera info
	std::cout << "Device name :: " << info.szDeviceDescription << std::endl;
	return errtem;
}

EdsError EDSCALLBACK mycanons::handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef, EdsVoid * inContext)
{

	//    console() << "Cinder-Canon :: Object Callback :: " << CanonEventToString(inEvent) << endl;
	switch (inEvent) {
		//case kEdsObjectEvent_DirItemRequestTransfer:
	case kEdsObjectEvent_DirItemCreated:
	{
		if (SingletonPhotoHandler)
		{
			std::cout << "Photo Taken. Calling photoTaken" << std::endl;
			EdsDirectoryItemRef dirItem = (EdsDirectoryItemRef)inRef;
			// NOTE: This is only called on success.
			// It should also be called on failure.
			SingletonPhotoHandler->photoTaken(dirItem, EDS_ERR_OK);
		}
		else
		{
			std::cout << "No photo callback. Ignoring." << std::endl;
			// This downloads to /tmp/canon
			// ((CinderCanon *)inContext)->downloadImage(inRef, NULL);
		}
		break;
	}
	}

	//// do something
	//switch (inEvent)
	//{ 
	//	case kEdsObjectEvent_DirItemRequestTransfer:
	//		downloadImage(inRef);
	//		break;
	//	default: 
	//		break;
	//} 

	// Object must be released
	if (inRef)
	{
		EdsRelease(inRef);
	}


	return EDS_ERR_OK;
}

EdsError EDSCALLBACK mycanons::handlePropertyEvent(EdsUInt32 inEvent, EdsUInt32 inPropertyID, EdsUInt32 inParam, EdsVoid * inContext)
{
	if (inPropertyID == kEdsPropID_Evf_OutputDevice) {
		std::cout << " ready for live viewing" << std::endl;
	}

	switch (inEvent)
	{
	case kEdsPropertyEvent_PropertyChanged:
		//fireEvent(controller, "get_Property", &inPropertyID);
		break;

	case kEdsPropertyEvent_PropertyDescChanged:
		//fireEvent(controller, "get_PropertyDesc", &inPropertyID);
		break;
	}


	return EDS_ERR_OK;
}

EdsError EDSCALLBACK mycanons::handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid * inContext)
{
	mycanons* controller = (mycanons *)inContext;

	switch (inEvent)
	{
	case kEdsStateEvent_Shutdown:
		controller->shutdown();
		break;
	}
	return EDS_ERR_OK;
}


mycanons::~mycanons()
{
	CloseListenTread();

}

EdsError mycanons::downloadImage(EdsDirectoryItemRef directoryItem, int positionTem)
{
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;
	// Get directory item information
	EdsDirectoryItemInfo dirItemInfo;
	err = EdsGetDirectoryItemInfo(directoryItem, &dirItemInfo);

	// Create file stream for transfer destination
	if (err == EDS_ERR_OK)
	{
		//std::string pathtem = "./data/" + std::string(dirItemInfo.szFileName);
		std::string pathtem = "./data/" + camSetName[positionTem] + std::to_string(pushButtonNum) + ".jpg";

		err = EdsCreateFileStream(pathtem.c_str(),
			kEdsFileCreateDisposition_CreateAlways,   //kEdsFileCreateDisposition_CreateNew //kEdsFileCreateDisposition_CreateAlways
			kEdsAccess_ReadWrite, &stream);
	}

	// Download image
	if (err == EDS_ERR_OK)
	{
		err = EdsDownload(directoryItem, dirItemInfo.size, stream);
	}

	// Issue notification that download is complete
	if (err == EDS_ERR_OK)
	{
		err = EdsDownloadComplete(directoryItem);
	}
	// Release stream
	if (stream != NULL)
	{
		EdsRelease(stream);
		stream = NULL;
	}
	return err;
}


EdsError mycanons::takePicture(EdsCameraRef* cameratem)
{
	EdsError errtem;

	errtem = EdsSendCommand(*cameratem, kEdsCameraCommand_PressShutterButton
		, kEdsCameraCommand_ShutterButton_Completely);
	errtem = EdsSendCommand(*cameratem, kEdsCameraCommand_PressShutterButton
		, kEdsCameraCommand_ShutterButton_OFF);

	if (errtem != EDS_ERR_OK)
	{
		std::cout << " error: " << errtem<<"   "<< CanonErrorToString(errtem)<< std::endl;


	}
	return errtem;
}

EdsError mycanons::detectCameras()
{
	EdsError errtem = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	
	// Get camera list
	errtem = EdsGetCameraList(&cameraList);
	// Get number of cameras
	if (errtem == EDS_ERR_OK)
	{
		errtem = EdsGetChildCount(cameraList, &camNum);
		if (camNum == 0)
		{
			errtem = EDS_ERR_DEVICE_NOT_FOUND;
			std::cout << camNum << "No camera are detected." << std::endl;
			return errtem;
		}
		else
		{
			std::cout <<camNum << "camera(s) are detected."<< std::endl;
		}
	}

	// Get camera retrieved
	if (errtem == EDS_ERR_OK)
	{
		cameraSet = new EdsCameraRef[camNum];
		cameraInfoSet = new EdsDeviceInfo[camNum];

		for (EdsUInt32 i = 0; i < camNum; i++)
		{
			// fetch cameras
			errtem = EdsGetChildAtIndex(cameraList, i, cameraSet+i);
			//errtem = EdsGetChildAtIndex(cameraList, i, &cameraSet[i]);

			// fetch camera info
			if (errtem == EDS_ERR_OK)
			{
				//getDeviceInfo(cameraSet[i]);
				errtem = EdsGetDeviceInfo(cameraSet[i], cameraInfoSet + i);
				if (errtem != EDS_ERR_OK)
				{
					std::cout << " Couldn't retrieve camera info of camera "<<i << std::endl;
					return errtem;
				}
				// print camera info
				std::cout << "Device "<< i << " name :: " << cameraInfoSet[i].szDeviceDescription << std::endl;

			}
		}	
	}

	// Release camera list
	if (cameraList != NULL)
	{
		EdsRelease(cameraList);
		cameraList = NULL;
	}
	return errtem;
}

EdsError mycanons::performAction()
{
	/** 打开会话*/
	EdsOpenSession(camera);

	int keystate = 0;
	keystate = GetKeyState(VK_SPACE);
	if (keystate < 0) // space 被按下
	{
		//endLiveView();

		//takePictureRef();
		takePicture(&camera);

		// 等待图像被存储下来
		Sleep(1000);

		do {
			keystate = GetKeyState(VK_SPACE);
			if (keystate < 0)
				continue;
			else
				break;
		} while (true);


		EdsVolumeRef cameraVol;
		getVolume(camera, &cameraVol);

		EdsDirectoryItemRef camearaDirItem;
		getDCIMFolder(cameraVol, &camearaDirItem,0);

		if (cameraVol)
		{
			EdsRelease(cameraVol);
		}
		if (camearaDirItem)
		{
			EdsRelease(camearaDirItem);
		}

		// 重新建立联系
		EdsCloseSession(camera);

		//startLiveView();
	}



	//downloadImage(EdsDirectoryItemRef directoryItem);

	return EDS_ERR_OK;

}

EdsError mycanons::shutdown()
{
	// 结束会话
	EdsCloseSession(camera);

	//Release Camera
	if (camera != NULL)
	{
		EdsRelease(camera);
		camera = NULL;
	}
	//Termination of SDK
	//Terminates use of the libraries.
	//Calling this function releases all resources allocated by the libraries.
	if (isSDKLoaded)
	{
		EdsTerminateSDK();
	}

	return EDS_ERR_OK;
}

UINT WINAPI mycanons::ListenThread(void* pParam)
{
	//When using the SDK from another thread in Windows, 
	// you must initialize the COM library by calling CoInitialize 
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	/** 得到本类的指针 */
	mycanons *pCamera = reinterpret_cast<mycanons*>(pParam);

	EdsError errtem = EDS_ERR_OK;

	///** 打开会话*/
	//errtem = EdsOpenSession(pCamera->camera);

	while (!pCamera->s_bExit)
	{
		Sleep(10);
		int keystate = 0;
		keystate = GetKeyState(VK_SPACE);
		if (keystate < 0) // space 被按下
		{
			//takePicture();
			do {
				keystate = GetKeyState(VK_SPACE);
				if (keystate < 0)
					continue;
				else
					break;
			} while (true);
		}
	}

	CoUninitialize();

	return 0;
}

bool mycanons::OpenListenThread()
{
	/** 检测线程是否已经开启了 */
	if (hThread != INVALID_HANDLE_VALUE)
	{
		/** 线程已经开启 */
		return false;
	}

	s_bExit = false;

	/** 线程ID */
	UINT threadId;
	/** 开启串口数据监听线程 */
	hThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!hThread)
	{
		return false;
	}
	/** 设置线程的优先级,高于普通线程 */
	if (!SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;

}

bool mycanons::CloseListenTread()
{
	if (hThread != INVALID_HANDLE_VALUE)
	{
		/** 通知线程退出 */
		s_bExit = true;

		/** 等待线程退出 */
		Sleep(10);

		/** 置线程句柄无效 */
		CloseHandle(hThread);
		hThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

void PhotoHandler::photoTaken(EdsDirectoryItemRef directoryItem, EdsError error)
{
}

void PhotoHandler::photoDownloaded(const std::string & downloadPath, EdsError error)
{
}

std::string PhotoHandler::photoDownloadDirectory()
{
	std::string savepath = "./";
	return savepath;
}
