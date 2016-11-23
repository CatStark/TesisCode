#pragma once

#ifndef FinalImage_H
#define FinalImage_H

#include "Patch.h"

class FinalImage
{
public:
	FinalImage(Mat &img, int, int, int);
    Mat selectSubset(Mat &originalImg, int, int);
    Mat placeRandomly(Patch, Mat &img);
    Mat textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, int, int);
    double msqe(Mat &target, Mat &patch);
    std::pair<double, Mat> getRandomPatch(std::vector<_patches> patchesList);
    Mat choseTypeTexture( Mat &img, Mat &img2, int backgroundPorcentage);

	std::vector<_patches> patchesList;
    _patches tmpPatch;
    Mat bestPatch;
    int posYTarget, posYPatch, posXPatch;
    int overlap,offset ;

    Mat newimg;
    double err;
    std::pair<double, Mat> bestError;
    double tempError;
    double minError;
    int width, height;
private:
};

#endif