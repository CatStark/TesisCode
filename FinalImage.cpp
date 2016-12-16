#include "FinalImage.h"


struct findRepeatedPatch
{
    double error;
    findRepeatedPatch(double error) : error(error) {}
    bool operator() (const Patch& m) const
    {
        return m.error == error;
    }
};

//Constructor
FinalImage::FinalImage(Mat &img, int y_expand, int x_expand, int windowSize)
{
	newimg = Mat::zeros(img.rows + y_expand  , img.cols + x_expand, CV_64FC3);
	newimg.convertTo(newimg, CV_8UC1);
	width = newimg.cols;
    height = newimg.rows;
    backgroundPorcentageTmp = 0;
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

Patch FinalImage::getRandomPatch(std::vector<Patch> patchesList)
{
    //return random error from list of best errors
    //If we don't do this, the chosen patches will look extremely similar
	std::vector<Patch> bestErrorsList;

    double tempError;
    Patch bestPatch; 
    double minError = 2.0; //Chose a value for acceptable error
    
    //Check that each new error stored in PatchesList is in fact new
    std::vector<Patch>::iterator isRepeatedElem;
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
    if ( bestErrorsList.size() != 0) {    
        bestPatch = bestErrorsList[i];
    }
    if (bestErrorsList.size() == 0)
        cout << "WARNING, empty list" << endl; //This should never happen

    return bestPatch;
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
            posXPatch += patch.width;
        }
        posXPatch = 0;
        posYPatch += patch.height;
    }
   
   return newimg;

}

Mat FinalImage::choseTypeTexture(Mat &img, Mat &img2, Patch &p, Grid &g, int x, int y) //Chose either background (0) or details (1) texutre
{
	if (g.grid[x][y] == 0){
		p.typeOfTexture = 1;
		return img;
	}
	else{
		p.typeOfTexture = 2;
		return img2;
	}
	
}

void FinalImage::addLinearBlending(Mat &target, Mat &patch, int posXPatch, int posYPatch) //LinearBlending
{
	double alpha = 0.6; double beta; double input = 0.5;
	Mat dst;
	int posX = posXPatch;
	int posY = posYPatch;

	if( input >= 0.0 && input <= 1.0 ) //Validate correct value for alpha
		{ alpha = input; }

	beta = ( 1.0 - alpha );
	addWeighted(target, alpha, patch, beta, 0.0, dst);

	Rect rect2(posX, posY, target.cols, target.rows);
	dst.copyTo(newimg(rect2));

}

Mat FinalImage::textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, int backgroundPorcentage, int detailsPorcentage)
{
	Patch newTarget(img); //Temporal target for new rows
	Patch bestP(img);

	Mat selectedTexture;

	overlap = patch.width / 6; 
	offset = patch.width - overlap; 
	posYPatch = posYTarget = 0;
	posXPatch = patch.width - overlap;
	gridSize = (newimg.cols/patch.width) * (newimg.rows/patch.height);

	//Size of grid
	gridX = (width / patch.width) + 1; //plus one because with the overlaping of patches, there is space for one more
	gridY = (height / patch.height) + 1;
	Grid grid(gridX, gridY); //Create grid
	grid.fill(backgroundPorcentage); 
	//poisson _poi;

	selectedTexture = choseTypeTexture(img, img2, patch, grid, 0,0); //create target
    target.image = selectSubset(selectedTexture, target.width, target.height); //Create a smaller subset of the original image 
    Rect rect(0,0, target.width, target.height);
    target.image.copyTo(newimg(rect));
    
	for (int patchesInY = 0; patchesInY < grid.grid[1].size(); patchesInY++)
   {
        for (int patchesInX = 1; patchesInX < grid.grid.size(); patchesInX++)
        {
            //Start comparing patches (until error is lower than tolerance)
            selectedTexture = choseTypeTexture(img, img2, patch, grid, patchesInX, patchesInY);
            for (int i = 0; i < 2000 ; i++) //This alue needs to be at least 50
            {
            	//Set image to the Patch
                patch.image = selectSubset(selectedTexture, patch.width, patch.height); //subselection from original texture

                //Create ROIs
                patch.roiOfPatch = patch.image(Rect(0, 0, overlap, patch.height));
                patch.roiOfTarget = target.image(Rect(offset, 0, overlap, target.height));
                patch.halfOfTarget = target.image(Rect(target.width/4, 0, target.width-(target.width/4), target.height));

                err = msqe(patch.roiOfTarget, patch.roiOfPatch);

                if (patchesInY > 0) //if is the second or bigger row
                {
                    patch.roiOfTopPatch = patch.image(Rect(0, 0, patch.width, overlap));
                    patch.roiOfBotTarget = newimg(Rect(posXPatch, posYPatch - overlap, patch.width, overlap));
                   
                    /*imshow("bot t", patch.roiOfBotTarget);
	       			imshow("top p", patch.roiOfTopPatch);*/
                    
                    err += msqe(patch.roiOfTopPatch, patch.roiOfBotTarget);
                    err = err/2; 
                }

                patch.error = err;
                _patchesList.push_back(patch);
                err = 0;
            }

            //chose random patch from best errors list
            bestP = getRandomPatch(_patchesList);


            Mat tmp = selectSubset(img, patch.width, patch.height);
             
	        Rect rect2(posXPatch, posYPatch, patch.width, patch.height);
	        tmp.copyTo(newimg(rect2));
	        //Mat tmp = newimg(Rect(posXPatch, posYPatch, patch.width, patch.height));
      
	       /* if (patchesInX - 1 >= 0 && grid.grid[patchesInX][patchesInY] == grid.grid[patchesInX-1][patchesInY])
	        {
	        	cout << "same type " << endl;
	       		addLinearBlending(bestP.roiOfTarget, bestP.roiOfPatch, posXPatch, posYPatch);
	       		
	       		if ( patchesInY - 1 >= 0 && grid.grid[patchesInX][patchesInY] == grid.grid[patchesInX][patchesInY-1]) {
	       			cout << "same type 2" << endl;
	       			addLinearBlending(bestP.roiOfBotTarget, bestP.roiOfTopPatch, posXPatch, posYPatch-overlap);
	       		}	
	        }
	       	else{*/
	       		cout << "different type " << endl;

				Mat src = bestP.image;
				Mat dst;

				dst = newimg(Rect(0, 0, posXPatch + patch.width, posYPatch + patch.height));
				//imshow("dst", dst);
				
				// Create an all white mask
	       		Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());

	       		// The location of the center of the src in the dst
	       		Point center;
	       		if (patchesInY == 0)
					center = Point(posXPatch + overlap*2, posYPatch + patch.height/2);
				else
					center = Point(posXPatch + overlap*2, posYPatch + overlap*2 );
				//cout << "value: "  << patch.height/2 << endl;

				// Seamlessly clone src into dst and put the results in output
				Mat normal_clone;
				Mat mixed_clone;

				seamlessClone(src, dst, src_mask, center, normal_clone, NORMAL_CLONE);
				normal_clone.copyTo(newimg(Rect(0, 0, normal_clone.cols, normal_clone.rows)));

	       //	} 
	       	

	        target.image = bestP.image;
            posXPatch += patch.width - overlap;
            _patchesList.clear();
		}
		posXPatch = patch.width - overlap;

		//New patch of the next row (new first target)
        posYPatch += patch.height - overlap;
        newTarget.roiOfBotTarget = newimg(Rect(0, posYPatch , patch.width, overlap));

        if (patchesInY < newimg.rows/patch.height)
       {
            for (int i = 0; i < 1000; i++)
            {
            	selectedTexture = choseTypeTexture(img, img2, patch, grid, 0, patchesInY+1);
                newTarget.image = selectSubset(selectedTexture, newTarget.width, newTarget.height); //subselection from original texture
                
                //Create ROIs
                newTarget.roiOfTopPatch = newTarget.image(Rect(0, 0, newTarget.width, overlap));   

                //Calculate errors
                err = msqe(newTarget.roiOfTopPatch, newTarget.roiOfBotTarget);

                newTarget.error = err;
                _patchesList.push_back(newTarget);

            }

            //chose best error
            bestP = getRandomPatch(_patchesList);
            newTarget.image = bestP.image;

            Rect rect2(0, posYPatch, patch.width, patch.height);
	        newTarget.image.copyTo(newimg(rect2));

            Mat src = newTarget.image;
			Mat dst;

			dst = newimg(Rect(0, 0, patch.width, posYPatch + patch.height));
			imshow("dst", dst);
			imshow("src", src);
			
			// Create an all white mask
       		Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());

       		// The location of the center of the src in the dst
			Point center(patch.width/2 , posYPatch );

			// Seamlessly clone src into dst and put the results in output
			Mat normal_clone;

			seamlessClone(src, dst, src_mask, center, normal_clone, NORMAL_CLONE);
			circle( normal_clone, center, 5.0, Scalar( 0, 50, 255 ), 1, 8 );
			//imshow("nor", normal_clone);
			normal_clone.copyTo(newimg(Rect(0, 0, normal_clone.cols, normal_clone.rows)));

            target = newTarget;
            _patchesList.clear();     
        }  
	}
	return newimg;
}