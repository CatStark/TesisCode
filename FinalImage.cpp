#include "FinalImage.h"
#include <iostream>

struct findRepeatedPatch
{
    double error;
    findRepeatedPatch(double error) : error(error) {}
    bool operator() (const _patches& m) const
    {
        return m.error == error;
    }
};

//Constructor
FinalImage::FinalImage(Mat &img, int y_expand, int x_expand, int windowSize)
{
	newimg = Mat::zeros(img.rows + y_expand  , img.cols + x_expand, CV_64FC3);
	width = newimg.cols;
    height = newimg.rows;
	cout << " ------------Output image created--------------" << endl;
}

Mat FinalImage::selectSubset(Mat &originalImg, int width_patch, int height_patch)
{
    //copy a sub matrix of X to Y with starting coodinate (startX,startY)
    // and dimension (cols,rows)
    int startX = rand() % (originalImg.cols - width_patch);
    int startY = rand() % (originalImg.rows - height_patch);
    Mat tmp = originalImg(cv::Rect(startX, startY, width_patch, height_patch)); 
    Mat subset;
    tmp.copyTo(subset);
    return subset;

}

double FinalImage::msqe(Mat &target, Mat &patch)
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

std::pair<double, Mat> FinalImage::getRandomPatch(std::vector<_patches> patchesList)
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

Mat FinalImage::placeRandomly(Patch patch, Mat &img)
{
    int posXPatch =  0;
    int posYPatch = 0;
    srand(time(NULL)); //Seed to get randmon patches


    for (int patchesInY = 0; patchesInY < newimg.rows/patch.height; patchesInY++)
   {
        for (int patchesInX = 0; patchesInX < newimg.cols/patch.width; patchesInX++)
        {
            patch.image = selectSubset(img, patch.width, patch.height); //subselection from original texture
            Rect rect2(posXPatch, posYPatch, patch.width, patch.height);
            patch.image.copyTo(newimg(rect2));
            imshow("p image ", patch.image);
            posXPatch += patch.width;
        }
        posXPatch = 0;
        posYPatch += patch.height;
    }
   
   return newimg;

}

Mat FinalImage::textureSynthesis(Patch patch, Patch target, Mat &img)
{
	Patch newTarget(img); //Temporal target for new rows

	overlap = patch.width / 6; 
	offset = patch.width - overlap; 
	posYPatch = posYTarget = 0;
	posXPatch = patch.width - overlap;

    target.image = selectSubset(img, target.width, target.height); //Create a smaller subset of the original image 
    Rect rect(0,0, target.width, target.height);
    target.image.copyTo(newimg(rect));
    
	for (int patchesInY = 0; patchesInY <= newimg.rows/patch.height; patchesInY++)
   {
        for (int patchesInX = 0; patchesInX < newimg.cols/patch.width; patchesInX++)
        {
            //Start comparing patches (until error is lower than tolerance)
            for (int i = 0; i < 1000 ; i++) //Compare 3 patches  
            {
            	//Set image to the Patch
                patch.image = selectSubset(img, patch.width, patch.height); //subselection from original texture

                //Create ROIs
                Mat roiOfPatch = patch.image(Rect(0, 0, overlap, patch.height));
                Mat roiOfTarget = target.image(Rect(offset, 0, overlap, target.height));

                err = msqe(roiOfTarget, roiOfPatch);

                if (patchesInY > 0) //if is the second or bigger row
                {
                    Mat roiOfTopPatch = patch.image(Rect(0, 0, patch.width, overlap));
                    Mat roiOfBotTarget = newimg(Rect(posXPatch, posYPatch, patch.width, overlap));

                    err += msqe(roiOfTopPatch, roiOfBotTarget);
                    err = err/2; 
                }
                    
                tmpPatch.error = err;
                tmpPatch.image = patch.image;
                patchesList.push_back(tmpPatch); 
                err = 0;
            }
            //chose random patch from best errors list
	        bestError = getRandomPatch(patchesList); 
	        tempError = bestError.first;
	        bestPatch = bestError.second;
	        cout << "best patch found " << endl;

	        Rect rect2(posXPatch, posYPatch, patch.width, patch.height);
	        bestPatch.copyTo(newimg(rect2));

	        target.image = bestPatch;
            posXPatch += patch.width - overlap;
            patchesList.clear();
		}
		posXPatch = patch.width - overlap;

		//New patch of the next row (new first target)
        posYPatch += patch.height - overlap;
        Mat roiOfBotTarget = newimg(Rect(0, posYPatch , patch.width, overlap));


        if (patchesInY < newimg.rows/patch.height)
       {
            for (int i = 0; i < 1000; i++)
            {
                newTarget.image = selectSubset(img, newTarget.width, newTarget.height); //subselection from original texture
                
                //Create ROIs
                Mat roiOfTopPatch = newTarget.image(Rect(0, 0, newTarget.width, overlap));   

                //Calculate errors
                err = msqe(roiOfTopPatch, roiOfBotTarget);

                tmpPatch.error = err;
                tmpPatch.image = newTarget.image;
                patchesList.push_back(tmpPatch);
            }

            //chose best error
            bestError = getRandomPatch(patchesList);
            tempError = bestError.first;
            bestPatch = bestError.second;
            newTarget.image = bestPatch;

            //newTarget.convertTo(newTarget, CV_64FC3);
            Rect rect2(0, posYPatch, patch.width, patch.height);
            newTarget.image.copyTo(newimg(rect2));

            target = newTarget;
            patchesList.clear();     
        }  
	}
	return newimg;
}