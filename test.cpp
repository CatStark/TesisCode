
// Texture Synthesis
//version 3
/*
I need to generate a list of "best errors" and then pick one randomly from there, 
otherwise the algorithm will end up picking always a very similar patch
*/


#include <iostream>
#include <math.h>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#define ATD at<double>
#define elif else if


using namespace cv;
using namespace std;

int width_patch;
int height_patch;
typedef struct _unfilled_point{
    Point2i pt;
    int fNeighbors;

}ufp;

typedef struct _Matches{
    Point2i pt;
    double err;
}matches;


typedef struct _patches{
    double error;
    Mat image;
}_patches;


struct findRepeatedPatch
{
    double error;
    findRepeatedPatch(double error) : error(error) {}
    bool operator() (const _patches& m) const
    {
        return m.error == error;
    }
};

// calculate horizontal gradient, img(i, j + 1) - img(i, j)
Mat 
getGradientXp(Mat &img){
    int height = img.rows;
    int width = img.cols;
    Mat cat = repeat(img, 1, 2);

    Rect roi = Rect(1, 0, width, height);
    Mat roimat = cat(roi);
    return roimat - img;
}

// calculate vertical gradient, img(i + 1, j) - img(i, j)
Mat 
getGradientYp(Mat &img){
    int height = img.rows;
    int width = img.cols;
    Mat cat = repeat(img, 2, 1);

    Rect roi = Rect(0, 1, width, height);
    Mat roimat = cat(roi);
    return roimat - img;
}

// calculate horizontal gradient, img(i, j - 1) - img(i, j)
Mat 
getGradientXn(Mat &img){
    int height = img.rows;
    int width = img.cols;
    Mat cat = repeat(img, 1, 2);

    Rect roi = Rect(width - 1, 0, width, height);
    Mat roimat = cat(roi);
    return roimat - img;
}

// calculate vertical gradient, img(i - 1, j) - img(i, j)
Mat 
getGradientYn(Mat &img){
    int height = img.rows;
    int width = img.cols;
    Mat cat = repeat(img, 2, 1);

    Rect roi = Rect(0, height - 1, width, height);
    Mat roimat = cat(roi);
    return roimat - img;
}

int
getLabel(int i, int j, int height, int width){
    return i * width + j;
}

// get Matrix A.
Mat
getA(int height, int width){

    Mat A = Mat::eye(height * width, height * width, CV_64FC1);
    A *= -4;
    Mat M = Mat::zeros(height, width, CV_64FC1);
    Mat temp = Mat::ones(height, width - 2, CV_64FC1);
    Rect roi = Rect(1, 0, width - 2, height);
    Mat roimat = M(roi);
    temp.copyTo(roimat);
    temp = Mat::ones(height - 2, width, CV_64FC1);
    roi = Rect(0, 1, width, height - 2);
    roimat = M(roi);
    temp.copyTo(roimat);
    temp = Mat::ones(height - 2, width - 2, CV_64FC1);
    temp *= 2;
    roi = Rect(1, 1, width - 2, height - 2);
    roimat = M(roi);
    temp.copyTo(roimat);

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            int label = getLabel(i, j, height, width);
            if(M.ATD(i, j) == 0){
                if(i == 0)  A.ATD(getLabel(i + 1, j, height, width), label) = 1;
                elif(i == height - 1)   A.ATD(getLabel(i - 1, j, height, width), label) = 1;
                if(j == 0)  A.ATD(getLabel(i, j + 1, height, width), label) = 1;
                elif(j == width - 1)   A.ATD(getLabel(i, j - 1, height, width), label) = 1;
            }elif(M.ATD(i, j) == 1){
                if(i == 0){
                    A.ATD(getLabel(i + 1, j, height, width), label) = 1;
                    A.ATD(getLabel(i, j - 1, height, width), label) = 1;
                    A.ATD(getLabel(i, j + 1, height, width), label) = 1;
                }elif(i == height - 1){
                    A.ATD(getLabel(i - 1, j, height, width), label) = 1;
                    A.ATD(getLabel(i, j - 1, height, width), label) = 1;
                    A.ATD(getLabel(i, j + 1, height, width), label) = 1;
                }
                if(j == 0){
                    A.ATD(getLabel(i, j + 1, height, width), label) = 1;
                    A.ATD(getLabel(i - 1, j, height, width), label) = 1;
                    A.ATD(getLabel(i + 1, j, height, width), label) = 1;
                }elif(j == width - 1){
                    A.ATD(getLabel(i, j - 1, height, width), label) = 1;
                    A.ATD(getLabel(i - 1, j, height, width), label) = 1;
                    A.ATD(getLabel(i + 1, j, height, width), label) = 1;
                }
            }else{
                    A.ATD(getLabel(i, j - 1, height, width), label) = 1;
                    A.ATD(getLabel(i, j + 1, height, width), label) = 1;
                    A.ATD(getLabel(i - 1, j, height, width), label) = 1;
                    A.ATD(getLabel(i + 1, j, height, width), label) = 1;
            }
        }
    }
    return A;
}

// Get the following Laplacian matrix
// 0  1  0
// 1 -4  1
// 0  1  0
Mat
getLaplacian(){
    Mat laplacian = Mat::zeros(3, 3, CV_64FC1);
    laplacian.ATD(0, 1) = 1.0;
    laplacian.ATD(1, 0) = 1.0;
    laplacian.ATD(1, 2) = 1.0;
    laplacian.ATD(2, 1) = 1.0;
    laplacian.ATD(1, 1) = -4.0; 
    return laplacian;
}

// Calculate b
// using convolution.
Mat
getB1(Mat &img1, Mat &img2, int posX, int posY, Rect ROI){
    Mat Lap;
    filter2D(img1, Lap, -1, getLaplacian());
    int roiheight = ROI.height;
    int roiwidth = ROI.width;
    Mat B = Mat::zeros(roiheight * roiwidth, 1, CV_64FC1);
    for(int i=0; i<roiheight; i++){
        for(int j=0; j<roiwidth; j++){
            double temp = 0.0;
            temp += Lap.ATD(i + ROI.y, j + ROI.x);
            if(i == 0)              temp -= img2.ATD(i - 1 + posY, j + posX);
            if(i == roiheight - 1)  temp -= img2.ATD(i + 1 + posY, j + posX);
            if(j == 0)              temp -= img2.ATD(i + posY, j - 1 + posX);
            if(j == roiwidth - 1)   temp -= img2.ATD(i + posY, j + 1 + posX);
            B.ATD(getLabel(i, j, roiheight, roiwidth), 0) = temp;
        }
    }
    return B;
}

// Calculate b
// using getGradient functions.
Mat
getB2(Mat &img1, Mat &img2, int posX, int posY, Rect ROI){
    Mat grad = getGradientXp(img1) + getGradientYp(img1) + getGradientXn(img1) + getGradientYn(img1);
    int roiheight = ROI.height;
    int roiwidth = ROI.width;
    Mat B = Mat::zeros(roiheight * roiwidth, 1, CV_64FC1);
    for(int i=0; i<roiheight; i++){
        for(int j=0; j<roiwidth; j++){
            double temp = 0.0;
            temp += grad.ATD(i + ROI.y, j + ROI.x);
            if(i == 0)              temp -= img2.ATD(i - 1 + posY, j + posX);
            if(i == roiheight - 1)  temp -= img2.ATD(i + 1 + posY, j + posX);
            if(j == 0)              temp -= img2.ATD(i + posY, j - 1 + posX);
            if(j == roiwidth - 1)   temp -= img2.ATD(i + posY, j + 1 + posX);
            B.ATD(getLabel(i, j, roiheight, roiwidth), 0) = temp;
        }
    }
    return B;
}

// Solve equation and reshape it back to the right height and width.
Mat
getResult(Mat &A, Mat &B, Rect &ROI){
    Mat result;
    solve(A, B, result);
    result = result.reshape(0, ROI.height);
    return  result;
}



// img1: 3-channel image, we wanna move something in it into img2.
// img2: 3-channel image, dst image.
// ROI: the position and size of the block we want to move in img1.
// posX, posY: where we want to move the block to in img2

Mat
poisson_blending(Mat &img1, Mat &img2, Rect ROI, int posX, int posY){

    int roiheight = ROI.height;
    int roiwidth = ROI.width;
    Mat A = getA(roiheight, roiwidth);

    // we must do the poisson blending to each channel.
    vector<Mat> rgb1;
    split(img1, rgb1);
    vector<Mat> rgb2;
    split(img2, rgb2);

    vector<Mat> result;
    Mat merged, res, Br, Bg, Bb;
    // For calculating B, you can use either getB1() or getB2()
    //Br = getB1(rgb1[0], rgb2[0], posX, posY, ROI);
    Br = getB2(rgb1[0], rgb2[0], posX, posY, ROI);
    res = getResult(A, Br, ROI);
    result.push_back(res);
    //cout<<"R channel finished..."<<endl;
    //Bg = getB1(rgb1[1], rgb2[1], posX, posY, ROI);
    Bg = getB2(rgb1[1], rgb2[1], posX, posY, ROI);
    res = getResult(A, Bg, ROI);
    result.push_back(res);
    //cout<<"G channel finished..."<<endl;
    //Bb = getB1(rgb1[2], rgb2[2], posX, posY, ROI);
    Bb = getB2(rgb1[2], rgb2[2], posX, posY, ROI);
    res = getResult(A, Bb, ROI);
    result.push_back(res);
    //cout<<"B channel finished..."<<endl;
    

    // merge the 3 gray images into a 3-channel image 
    merge(result,merged);
    return merged; 
}

Mat selectSubset(const Mat &originalImg)
{
    //copy a sub matrix of X to Y with starting coodinate (startX,startY)
    // and dimension (cols,rows)
    int startX = rand() % (originalImg.rows - width_patch);
    int startY = rand() % (originalImg.cols - height_patch);
    Mat tmp = originalImg(cv::Rect(startX, startY, width_patch, height_patch)); 
    Mat subset;
    tmp.copyTo(subset);
    return subset;

}

double msqe(Mat &target, Mat &patch)
{
    int i, j;
    double eqm, tmpEqm = 0;
    int height = target.rows;
    int width = target.cols;
    uint8_t* pixelPtrT = (uint8_t*)target.data;
    uint8_t* pixelPtrP = (uint8_t*)patch.data;
    Scalar_<uint8_t> bgrPixelT;
    int cnT = target.channels();
    int cnP = patch.channels();
    double valTarget, valPatch = 0;
    double R, G, B = 0;

    for ( i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {


           /* valTarget += pixelPtrT[i*target.cols*cnT + j*cnT + 0]; // B
            valTarget += pixelPtrT[i*target.cols*cnT + j*cnT + 1]; // G
            valTarget += pixelPtrT[i*target.cols*cnT + j*cnT + 2]; // R

            valPatch += pixelPtrP[i*patch.cols*cnP + j*cnP + 0]; // B
            valPatch += pixelPtrP[i*patch.cols*cnP + j*cnP + 1]; // G
            valPatch += pixelPtrP[i*patch.cols*cnP + j*cnP + 2]; // R

            eqm += sqrt((valTarget - valPatch) * (valTarget - valPatch));*/

            B = pixelPtrT[i*target.cols*cnT + j*cnT + 0]; // B
            B -= pixelPtrP[i*patch.cols*cnP + j*cnP + 0]; // B

            G = pixelPtrT[i*target.cols*cnT + j*cnT + 1]; // G
            G -= pixelPtrP[i*patch.cols*cnP + j*cnP + 1]; // G

            R = pixelPtrT[i*target.cols*cnT + j*cnT + 2]; // R
            R -= pixelPtrP[i*patch.cols*cnP + j*cnP + 2]; // R

            tmpEqm = sqrt((B + G + R) * (B + G + R));
            eqm += tmpEqm;

            valTarget = 0;
            valPatch = 0;
          
        }
    }

    eqm /= height * width;
    return eqm;
}

std::pair<double, Mat> getRandomPatch(std::vector<_patches> patchesList)
{
    //return random error from list of best errors
    //If we don't do this, the chosen patches will look extremely similar
    std::vector<_patches> bestErrorsList;
    double tempError;
    Mat bestPatch; 
    double minError = 50.0; //Chose a value for acceptable error
    //Check that each new error stored in PatchesList is in fact new
    std::vector<_patches>::iterator isRepeatedElem;
    bool repeatedElem;
    while (bestErrorsList.size() < 50)
    {
        for (int i = 0; i < patchesList.size(); i++)
        {
            isRepeatedElem = find_if(bestErrorsList.begin(), bestErrorsList.end(), findRepeatedPatch(patchesList[i].error));
            if (isRepeatedElem != bestErrorsList.end()) repeatedElem = true;
            else repeatedElem = false;

            if (patchesList[i].error < minError && !repeatedElem )
            {
                bestErrorsList.push_back(patchesList[i]);
            }
        }
        minError += 3;
    }

    //cout << "final size" << bestErrorsList.size() << endl;

    int i = rand() % (bestErrorsList.size() - 1); //return random error from list of best errors
    if ( bestErrorsList.size() != 0)
    {
        tempError = bestErrorsList[i].error;
        bestPatch = bestErrorsList[i].image;  
    }
    if (bestErrorsList.size() == 0)
        cout << "WARNING, empty list" << endl; //This should never happen

    return make_pair(tempError, bestPatch);
}

//expand gray image 
Mat expandImage(const Mat &img, int x_expand, int y_expand, int windowSize){

    srand(time(NULL)); //Seed to get randmon patches
    std::vector<_patches> patchesList;
    _patches tmpPatch;
    Mat bestPatch;
    int posYTarget = 0;
    int posYPatch = 0;
    int overlap = width_patch / 6; //7,5
    int offset = width_patch - overlap; //22,5
    int posXPatch = width_patch - overlap;

    Mat newimg = Mat::zeros(img.rows + y_expand , img.cols + x_expand , CV_64FC3);
    cout << "overlap " << overlap  << endl;
    double err = 0;
    std::pair<double, Mat> bestError;
    double tempError;
    double minError = 0;
    
    //Create target
    Rect rect(0,0, width_patch, height_patch);
    Mat target = selectSubset(img); //Create a smaller subset of the original image 
    target.copyTo(newimg(rect));

    //Create Mats
    Mat patch;
    Mat subsetPatch;
    
   //Copy of first targets of row
    Mat firstTarget, newTarget;
    firstTarget = target;
    //while free space

    for (int patchesInY = 0; patchesInY <=  newimg.cols/width_patch; patchesInY++)
   {
        for (int patchesInX = 0; patchesInX < newimg.cols/width_patch; patchesInX++)
        {
            //Start comparing patches (until error is lower than tolerance)
            for (int i = 0; i < 1000 ; i++) //Compare 3 patches         
            {
                //Create Patch
                patch = selectSubset(img); //subselection from original texture
                
                //Create ROIs
                Mat roiOfPatch = patch(Rect(0, 0, overlap, height_patch));
                Mat roiOfTarget = target(Rect(offset, 0, overlap, height_patch));
                err = msqe(roiOfTarget, roiOfPatch);

                if (patchesInY > 0) //if is the second or bigger row
                {
                    Mat roiOfTopPatch = patch(Rect(0, 0, width_patch, overlap));
                    Mat roiOfBotTarget = newimg(Rect(posXPatch, posYPatch, width_patch, overlap));

                    err += msqe(roiOfTopPatch, roiOfBotTarget);
                    err = err/2; 
                }
                    
                tmpPatch.error = err;
                tmpPatch.image = patch;
                patchesList.push_back(tmpPatch); 
                err = 0;
            } 
            
            //chose rando patch from best errors list
            bestError = getRandomPatch(patchesList); 
            tempError = bestError.first;
            bestPatch = bestError.second;

            imshow("target", target);
            imshow("best", bestPatch);

            bestPatch.copyTo(newimg(Rect(posXPatch, posYPatch, width_patch, height_patch)));

            target = bestPatch;
            posXPatch += width_patch - overlap;
            patchesList.clear();
        }

        posXPatch = width_patch - overlap;

        //New patch of the next row (new first target)
        posYPatch += height_patch - overlap;
        Mat roiOfBotTarget = newimg(Rect(0, posYPatch , width_patch, overlap));
        
        if (patchesInY < newimg.cols/width_patch)
        {
            for (int i = 0; i < 1000; i++)
            {
                newTarget = selectSubset(img); //subselection from original texture
                
                //Create ROIs
                Mat roiOfTopPatch = newTarget(Rect(0, 0, width_patch, overlap));   

                //Calculate errors
                err = msqe(roiOfTopPatch, roiOfBotTarget);

                tmpPatch.error = err;
                tmpPatch.image = newTarget;
                patchesList.push_back(tmpPatch);
            }

            //chose best error
            bestError = getRandomPatch(patchesList);
            tempError = bestError.first;
            bestPatch = bestError.second;
            newTarget = bestPatch;

            //newTarget.convertTo(newTarget, CV_64FC3);
            Rect rect2(0, posYPatch, width_patch, height_patch);
            newTarget.copyTo(newimg(rect2));

            target = newTarget;
            patchesList.clear();     
        }  
    }

    return newimg;
}

Mat placeRandomly(const Mat &img, int x_expand, int y_expand, int windowSize)
{
    Mat newimg = Mat::zeros(img.rows + y_expand , img.cols + x_expand , CV_64FC3);
    int width = newimg.cols;
    int height = newimg.rows;
    int posXPatch =  0;
    int posYPatch = 0;
    //srand(time(NULL)); //Seed to get randmon patches

    Mat patch;

    for (int patchesInY = 0; patchesInY < newimg.rows/width_patch; patchesInY++)
   {
        for (int patchesInX = 0; patchesInX < newimg.cols/width_patch; patchesInX++)
        {
            patch = selectSubset(img); //subselection from original texture
            Rect rect2(posXPatch, posYPatch, width_patch, height_patch);
            patch.copyTo(newimg(rect2));
            posXPatch += width_patch;
        }
        posXPatch = 0;
        posYPatch += height_patch;
    }
   
   return newimg;

}

int main( int argc, char** argv ){

    int option = 0;
    long start, end;
    start = clock();
    srand(time(NULL)); //Seed to get randmon patches

    // Extend  gray image.
    Mat img;
    //img = imread("Moon.jpg");
    img = imread("161.jpg");
    //img = imread("D34.jpg");
    //img = imread("rice.jpg");
    //img = imread("text.bmp");
    imshow("original ", img);
    //img.convertTo(img, CV_64FC3);
    cout << "or rows " << img.rows << " or cols " << img.cols << endl; 
    Mat result;
    width_patch = img.cols/ 4;
    height_patch = img.rows/4;
    cout << "width_patch " << width_patch << " height_patch " << height_patch << endl; 
    cout << "1. Random    2. Image Quilting " << endl;
    cin >> option;
    if (option == 1)
        result = placeRandomly(img, 50, 50, 10);
    else if (option == 2)
        result = expandImage(img, img.rows, img.cols, 10);
    result.convertTo(result, CV_8UC1);
    imshow("Final", result);

    end = clock();
    cout<<"used time: "<<((double)(end - start)) / CLOCKS_PER_SEC<<" second"<<endl;
    
    waitKey(0);
    return 0;
}