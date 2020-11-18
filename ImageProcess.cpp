#include "ImageProcess.h"

/**************/
/* function: unzip a video to a series of images
/* parameters: 
/***************/
bool videoUnzip(const string& videopath, const string& savepath)
{
	VideoCapture video(videopath);
	if (!video.isOpened())
	{
		std::cout << "video is not open" << std::endl;
		return false;
	}

	//读取视频帧数
	double  frameCount = video.get(CAP_PROP_FRAME_COUNT);
	cout << "Numbers of frames in the input video files: " << frameCount << endl;

	//读取视频帧率
	double rate = video.get(CAP_PROP_FPS);
	cout << "frame rate of the input video file is : " << rate << endl;

	// 交互设计解压模式
	cout << "Set the number of frames you want to get, enter 1 skip this step " << endl;

	int desFramsNo = 0;

	bool IsGetRightNo(false);
	while (!IsGetRightNo)
	{
		cin >> desFramsNo;
		if (desFramsNo != 0)
			IsGetRightNo = true;
	}

	//当前视频帧
	Mat frame;
	//每一帧之间的延时
	int delay = static_cast<int>(frameCount / desFramsNo);

	bool IsStop(false);

	int countIndex = 0;

	while (!IsStop)
	{
		video >> frame;

		if (frame.empty())//如果某帧为空则退出循环
			break;

		if (countIndex % delay == 0)
		{
			imwrite(savepath + to_string(countIndex) + ".jpg", frame);
			cout << "save image " << to_string(countIndex) << endl;
		}
		countIndex++;

	}

	return true;

}


/**************/
/* function: transform a 3d render image to two 
             seperated 2d left and right images
/* parameters: mode means the formate of input image
               0 representes side-by-side formate
			   1 representes top-by-bottom
/***************/
void image3dto2d(Mat& inputImage, int mode, Mat& outputLeftIm, Mat& outputRightIm)
{
	// separete images
	int Width = inputImage.cols;
	int Hight = inputImage.rows;

	Rect leftFrameSize(0, 0, Width / 2, Hight);
	Rect rightFrameSize(Width / 2, 0, Width / 2, Hight);

	Mat leftIm_tem, rightIm_tem;
	inputImage(leftFrameSize).copyTo(leftIm_tem);
	inputImage(rightFrameSize).copyTo(rightIm_tem);
	
	resize(leftIm_tem,  outputLeftIm,  Size(Width, Hight), 0, 0, INTER_LINEAR);
	resize(rightIm_tem, outputRightIm, Size(Width, Hight), 0, 0, INTER_LINEAR);

}


/**************/
/* function: render a 3d image into a 
             rendered 3d image which can
			 stereo vision can be displayed
/* parameters: mode means the formate of input image
			   0 representes side-by-side formate
			   1 representes top-by-bottom
/***************/
void render3d(Mat& inputim,int mode, Mat& outputim)
{
	// separete images
	int Width = inputim.cols;
	int Hight = inputim.rows;

	// render
	outputim = Mat(Size(Width, Hight), CV_8UC3);
	for (int i = 0; i < Hight; i++)
	{

		for (int j = 0; j < Width; j++)
		{
			if (j % 2 == 0)
			{
				outputim.at<cv::Vec3b>(i, j)[0] = inputim.at<cv::Vec3b>(i, j / 2)[0];
				outputim.at<cv::Vec3b>(i, j)[1] = inputim.at<cv::Vec3b>(i, j / 2)[1];
				outputim.at<cv::Vec3b>(i, j)[2] = inputim.at<cv::Vec3b>(i, j / 2)[2];
			}
			else
			{
				outputim.at<cv::Vec3b>(i, j)[0] = inputim.at<cv::Vec3b>(i, Width / 2 + j / 2)[0];
				outputim.at<cv::Vec3b>(i, j)[1] = inputim.at<cv::Vec3b>(i, Width / 2 + j / 2)[1];
				outputim.at<cv::Vec3b>(i, j)[2] = inputim.at<cv::Vec3b>(i, Width / 2 + j / 2)[2];
			}
		}
	}
		

}
