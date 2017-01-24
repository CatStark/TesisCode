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

Mat FinalImage::choseTypeTexture(Mat &img, Mat &img2, Mat &img3, Patch &p, Grid &g, int x, int y) //Chose either background (0) or details (1) texutre
{
	if (g.grid[x][y] == 0){
		p.typeOfTexture = 0;
		return img;
	}
	else if (g.grid[x][y] == 1){
		p.typeOfTexture = 1;
		return img2;
	}
	else{
		p.typeOfTexture = 2;
		return img3;
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
	imshow("dst blend", dst);

	Rect rect2(posX, posY, target.cols, target.rows);
	dst.copyTo(newimg(rect2));

}

Mat FinalImage::textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, Mat &img3, int backgroundPorcentage, int detailsPorcentage)
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

	selectedTexture = choseTypeTexture(img, img2, img3, patch, grid, 0,0); //create target
    target.image = selectSubset(selectedTexture, target.width, target.height); //Create a smaller subset of the original image 
    Rect rect(0,0, target.width, target.height);
    target.image.copyTo(newimg(rect));
    
    
	for (int patchesInY = 0; patchesInY < /*grid.grid[1].size()*/2; patchesInY++)
   {
        for (int patchesInX = 1; patchesInX < grid.grid.size(); patchesInX++)
        {
            //Start comparing patches (until error is lower than tolerance)
            //Comment this just for testing new method (fill everything with background and add details later)
            selectedTexture = choseTypeTexture(img, img2, img3, patch, grid, patchesInX, patchesInY);
            
            for (int i = 0; i < 500 ; i++) //This alue needs to be at least 50
            {
            	//Set image to the Patch
            	//if (patch.typeOfTexture == 0)
                	patch.image = selectSubset(img, patch.width, patch.height); //subselection from original texture
      
                //cout << "ok " << endl;
                //Create ROIs
                patch.roiOfPatch = patch.image(Rect(0, 0, overlap, patch.height));
                patch.roiOfTarget = target.image(Rect(offset, 0, overlap, target.height));
                patch.halfOfTarget = target.image(Rect(target.width/4, 0, target.width-(target.width/4), target.height));

                err = msqe(patch.roiOfTarget, patch.roiOfPatch);

                if (patchesInY > 0) //if is the second or bigger row
                {
                    patch.roiOfTopPatch = patch.image(Rect(0, 0, patch.width, overlap));
                    patch.roiOfBotTarget = newimg(Rect(posXPatch, posYPatch - overlap, patch.width, overlap));
                    
                    err += msqe(patch.roiOfTopPatch, patch.roiOfBotTarget);
                    err = err/2; 
                }

                patch.error = err;
                _patchesList.push_back(patch);
                err = 0;
            }
            cout << "test " << endl;

            //chose random patch from best errors list
            bestP = getRandomPatch(_patchesList);
            Mat tmp = selectSubset(img, patch.width, patch.height);
             
	        Rect rect2(posXPatch, posYPatch, patch.width, patch.height);
	        tmp.copyTo(newimg(rect2));

	        Mat src;
			Mat dst;
			// Seamlessly clone src into dst and put the results in output
			Mat normal_clone;
			Mat mixed_clone;

			src = bestP.image;
			dst = newimg(Rect(0, 0, posXPatch + patch.width, posYPatch + patch.height));
				
			// Create an all white mask
	       	Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());
      
	     	if(patchesInX - 1 >= 0 && grid.grid[patchesInX][patchesInY] == 0)
	        {	

	       		// The location of the center of the src in the dst
	       		Point center;
	       		if (patchesInY == 0) //First row, don't change on vertical axis (Y)
					center = Point(posXPatch + overlap*2, posYPatch + patch.height/2);
				else
					center = Point(posXPatch + overlap*2, posYPatch + overlap*2 );

				seamlessClone(src, dst, src_mask, center, normal_clone, NORMAL_CLONE);			
				normal_clone.copyTo(newimg(Rect(0, 0, normal_clone.cols, normal_clone.rows)));
	        }
	      
	        target.image = bestP.image;
            posXPatch += patch.width - overlap;
            _patchesList.clear();
		}
		posXPatch = patch.width - overlap;

		cout << "test2 " << endl;
		//New patch of the next row (new first target)
        posYPatch += patch.height - overlap;
        newTarget.roiOfBotTarget = newimg(Rect(0, posYPatch , patch.width, overlap));

        if (patchesInY < newimg.rows/patch.height) //new target on next row
       {
            for (int i = 0; i < 100; i++)
            {
            	//selectedTexture = choseTypeTexture(img, img2, patch, grid, 0, patchesInY+1);
                newTarget.image = selectSubset(img, newTarget.width, newTarget.height); //subselection from original texture
                
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
			
			// Create an all white mask
       		Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());

       		// The location of the center of the src in the dst
			Point center(patch.width/2 , posYPatch + (overlap*2));

			// Seamlessly clone src into dst and put the results in output
			Mat normal_clone;

			seamlessClone(src, dst, src_mask, center, normal_clone, NORMAL_CLONE);
			//circle( normal_clone, center, 5.0, Scalar( 0, 50, 255 ), 1, 8 );
			//imshow("nor", normal_clone);
			normal_clone.copyTo(newimg(Rect(0, 0, normal_clone.cols, normal_clone.rows)));

            target = newTarget;
            _patchesList.clear();     
        }  
	}

	posYPatch = 0;
	posXPatch = patch.width - overlap;
	for (int patchesInY = 0; patchesInY < grid.grid[1].size(); patchesInY++)
   {
        for (int patchesInX = 1; patchesInX < grid.grid.size(); patchesInX++)
        {
        	if (grid.grid[patchesInX][patchesInY] == 1)
        	{
        		cout << "details " << endl;
	        	for (int i = 0; i < 100 ; i++) //This alue needs to be at least 50
	            {
	            	//Set image to the Patch
	                patch.image = selectSubset(img2, patch.width, patch.height); //subselection from original texture
	                //patch.image = img2;

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
		        imshow("tmp", tmp);

		        Mat src;
				Mat dst;
				// Seamlessly clone src into dst and put the results in output
				Mat normal_clone;
				Mat mixed_clone;

				src = bestP.image;
				dst = newimg(Rect(0, 0, posXPatch + patch.width, posYPatch + patch.height));
					
				// Create an all white mask
		       	Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());

		       	//GRABCUT

				// define bounding rectangle
			    int border = 15;
			    int border2 = border + border;
			    cv::Rect rectangle(border, border, bestP.image.cols-border2, bestP.image.rows-border2);

			    cv::Mat result; // segmentation result (4 possible values)
			    cv::Mat bgModel,fgModel; // the models (internally used)
			 

			    // GrabCut segmentation
			    cv::grabCut(bestP.image,    // input image
			        result,   // segmentation result
			        rectangle,// rectangle containing foreground 
			        bgModel,fgModel, // models
			        1,        // number of iterations
			        cv::GC_INIT_WITH_RECT); // use rectangle
			    
			    // Get the pixels marked as likely foreground
			    cv::compare(result, GC_PR_FGD, result, CMP_EQ);
			    
			    // Generate output image
			    cv::Mat foreground(bestP.image.size(),CV_8UC3,cv::Scalar(0,0,0));
			    bestP.image.copyTo(foreground,result); // bg pixels not copied
			 
			 	cv::rectangle(bestP.image, rectangle, cv::Scalar(255,255,255),1);
			    imshow("Image",bestP.image);
			 
			    // display result
			    imshow("Segmented Image",foreground);
			    //END GRABCUT

			    //Convert to BGRA
			    Mat thr;
			    Mat dst2 = Mat(foreground.cols, foreground.rows, CV_8UC4);

			    //Grayscaling
				cvtColor(foreground,tmp,CV_BGR2GRAY);

				//Thresholding
				threshold(tmp,thr,100,255,0);

				//Splitting & adding Alpha
				vector<Mat> channels;   // C++ version of ArrayList<Mat>
				split(foreground, channels);   // Automatically splits channels and adds them to channels. The size of channels = 3
				channels.push_back(thr);   // Adds mask(alpha) channel. The size of channels = 4	

				//Mat rgb[3];
				//split(foreground,rgb);

				// 5. Merging
				merge(channels, dst2);   // dst is created with 4-channel(BGRA).

				//Mat rgba[4]={rgb[0],rgb[1],rgb[2],thr};
				//merge(rgba,4,dst2);
				//imwrite("nue.png",dst2);
				imshow("dst2", dst2);
				//End convertion
				
				cout << "posXPatch " << posXPatch << endl; 
				Point location(posXPatch, posYPatch);
				Mat output;
			    dst.copyTo(output);

				  // start at the row indicated by location, or at row 0 if location.y is negative.
				  for(int y = std::max(location.y , 0); y < dst.rows; ++y)
				  {
				    int fY = y - location.y; // because of the translation

				    // we are done of we have processed all rows of the foreground image.
				    if(fY >= dst2.rows)
				      break;

				    // start at the column indicated by location, 

				    // or at column 0 if location.x is negative.
				    for(int x = std::max(location.x, 0); x < dst.cols; ++x)
				    {
				      int fX = x - location.x; // because of the translation.

				      // we are done with this row if the column is outside of the foreground image.
				      if(fX >= dst2.cols)
				        break;

				      // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
				      double opacity = ((double)dst2.data[fY * dst2.step + fX * dst2.channels() + 3])/ 255.;


				      // and now combine the background and foreground pixel, using the opacity, 

				      // but only if opacity > 0.
				      for(int c = 0; opacity > 0 && c < output.channels(); ++c)
				      {
				        unsigned char foregroundPx =
				          dst2.data[fY * dst2.step + fX * dst2.channels() + c];
				        unsigned char backgroundPx =
				          dst.data[y * dst.step + x * dst.channels() + c];
				        output.data[y*output.step + output.channels()*x + c] =
				          backgroundPx * (1.-opacity) + foregroundPx * opacity;
				      }
				    }
				  }

			    //imshow("test png", output);
			    //imwrite("testPNG.png", output);

			    output.copyTo(newimg(Rect(0,0, output.cols, output.rows)));
        	}
        	posXPatch += patch.width - overlap;
        }
        posXPatch = patch.width - overlap;
		//New patch of the next row (new first target)
       posYPatch += patch.height - overlap;
    }
	
    imwrite("final.png", newimg);
	return newimg;
}