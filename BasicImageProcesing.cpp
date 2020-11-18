#include <iostream>
#include "ImageProcess.h"

int main_ss()
{

  /*  string imagepath = "./data/realtest/";
    string savepath = "./data/realtest/result/";

    int imnum = 15;

    for (int i = 1; i < imnum; i++)
    {
        Mat imL, imR;
        Mat originalIm = imread(imagepath + to_string(i) + ".jpg");

        image3dto2d(originalIm, 0, imL, imR);

        imwrite(savepath + "left" + to_string(i)+  ".jpg",imL);
        imwrite(savepath + "right" + to_string(i) + ".jpg", imR);

   
    }*/

    namedWindow("showtest", WINDOW_AUTOSIZE); //WINDOW_AUTOSIZE, WINDOW_NORMAL


    string imagepath = "./data/test1.jpg";
    string savepath = "./data/";

    Mat im = imread(imagepath);
    Mat outim;
    render3d(im, 0, outim);

    Mat out1;
    resize(outim, out1, Size(3840, 2160), 0, 0, INTER_LINEAR);

    imwrite(savepath + "test1out1.jpg", out1);

    imshow("showtest", outim);
    waitKey();


    return 0;
}


