#pragma once

#ifndef patch_H
#define patch_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;
using namespace std;

typedef struct _patches
	{
    	double error;
    	Mat image;
    }_patches;

class Patch
{
public:
	Patch(Mat &img);
	Patch();
	int width, height;
	Mat image; //new image on the patch
	
private:
};

#endif
