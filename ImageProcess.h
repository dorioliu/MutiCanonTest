/***********************************
/@ some small demos will usually be 
    used in daily programming
/@ author: liuwei
***********************************/
#pragma once

#ifndef _IMAGEPROCESS_H
#define _IMAGEPROCESS_H

#include <opencv.hpp>
using namespace std;
using namespace cv;

/*
@ function: video unzip function to extract images from a video
@ paras: para 1 is a input video path including a video name and postfix
         para 2 is a output path to save extracted images
*/
bool videoUnzip(const string& videopath, const string& savepath);

void image3dto2d(Mat& inputImage, int mode, Mat& outputLeftIm, Mat& outputRightIm);

void render3d(Mat& inputim, int mode, Mat& outputim);

#endif // !_IMAGEPROCESS_H
