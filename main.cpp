#include <iostream>
//#include "ImageProcess.h"
#include "canoncamera.h"



int main() 
{
	mycanons *canonSets = new mycanons;

	//canonSets->takePicturesByMultiThread();


	while (1)
	{
		//Sleep(2);
		canonSets->takePicturesFromCams();
 		//canonSets->listenKeyEvent();
 	}

	delete canonSets;

	return 0;

}
