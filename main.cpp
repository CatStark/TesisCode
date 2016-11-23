#include "FinalImage.h"

#include <iostream>
#include <math.h>
#include <algorithm>

#define ATD at<double>
#define elif else if

int main( int argc, char** argv ){

    int option = 0;
    long start, end;
    start = clock();
    srand(time(NULL)); //Seed to get randmon patches
    
    // Create empty output image.
    Mat InputImg; //Background
    Mat InputImg2; //Details
    Mat result;
    
    //Load input images
    //img = imread("Moon.jpg");
    InputImg = imread("leaves.jpg");
    InputImg2 = imread("flowers.jpg");
    int backgroundPorcentage, detailsPorcentage = 0;

    imshow("original ", InputImg);

    //Create first patch from img
    Patch _patch(InputImg); 

    //Create target
    Patch _target(InputImg);


    //Create empty output texture
    FinalImage _finalImage(InputImg, InputImg.rows, InputImg.cols, 10);
   
    

    cout << "1. Random    2. Image Quilting " << endl;
    cin >> option;
    if (option == 1)
        result =  _finalImage.placeRandomly(_patch, InputImg);
    else if (option == 2)
    {
        cout << "How much porcentage you want to give to the Background (0 - 100) " << endl;
        //cin >> backgroundPorcentage;
        backgroundPorcentage = 60; //hardcoded just for debugging 
        cout << "How much porcentage you want to give to the details ( 0 - 100) " << endl;
        //cin >> detailsPorcentage;
        detailsPorcentage = 40;
        //TODO verification that background and details sums to 100%
        result = _finalImage.textureSynthesis(_patch, _target, InputImg, InputImg2, backgroundPorcentage, detailsPorcentage);
    }
   /* else if (option == 3)
        result = img;*/
    result.convertTo(result, CV_8UC1);
    imshow("Final", result);
    //imshow("img", img);

    end = clock();
    cout<<"used time: "<<((double)(end - start)) / CLOCKS_PER_SEC<<" second"<<endl;
    
    waitKey(0);
    return 0;
}