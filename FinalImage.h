#pragma once

#ifndef FinalImage_H
#define FinalImage_H

#include "Grid.h"

class FinalImage
{
public:
	FinalImage(Mat &img, int, int, int);
    Mat selectSubset(Mat &originalImg, int, int);
    Mat placeRandomly(Patch, Mat &img);
    Mat textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, int, int);
    double msqe(Mat &target, Mat &patch);
    Patch getRandomPatch(std::vector<Patch> patchesList);
    Mat choseTypeTexture( Mat &img, Mat &img2, int backgroundPorcentage, int gridSize, Patch &p);
    void addLinearBlending(Mat &target, Mat &patch, int posXPatch, int posYPatch);

    std::vector<Patch> _patchesList;
    _patches tmpPatch;
    int posYTarget, posYPatch, posXPatch;
    int overlap,offset ;

    Mat newimg;
    double err;
    std::pair<double, Mat> bestError;
    double tempError;
    double minError;
    int width, height;
    int backgroundPorcentageTmp;
    int gridSize, gridX, gridY;
private:
};

#endif