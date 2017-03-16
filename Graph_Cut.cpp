/*
Code is released under Simplified BSD License.
-------------------------------------------------------------------------------
Copyright 2014 Nghia Ho. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY NGHIA HO ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NGHIA HO OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Nghia Ho.
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cassert>
#include <vector>
#include <math.h>
#include "maxflow/graph.h"
#include "Graph_Cut.h"

using namespace std;
using namespace cv;

typedef Graph<int,int,int> GraphType;

/*Mat selectSubset(Mat &originalImg, int width_patch, int height_patch)
{
    int startX = rand() % (originalImg.cols - width_patch);
    int startY = rand() % (originalImg.rows - height_patch);
    Mat tmp = originalImg(cv::Rect(startX, startY, width_patch, height_patch)); 
    Mat subset;
    tmp.copyTo(subset);
    return subset;

}*/

//int main(int argc, char **argv)
void Graph_Cut::GC()
{
    /*long start = clock();
    srand(time(NULL)); //Seed to get randmon patches
    int orientation = 1; // 1 -> image next to the other 2 -> image below the other 3-> both
    int sizeOfPatch;

    Mat A_ = imread("triangle2.jpg");

     if (A_.cols/2 < A_.rows/2)
        sizeOfPatch = A_.cols/2;
    else
        sizeOfPatch = A_.rows/2;

    Mat A = selectSubset(A_, sizeOfPatch, sizeOfPatch);
    A = A_;
    assert(A.data);
    cout <<"A rows: " << A.rows << " A cols: " << A.cols << endl;

    Mat B_, B;
    B_ = imread("triangle.jpg");
    B = selectSubset(B_, sizeOfPatch, sizeOfPatch);
    B = B_;
    cout <<"B rows " << B.rows << " B cols " << B.cols << endl;

    Mat graphcut;
    Mat graphcut_and_cutline;

    int overlap_width = A.cols/2;
    int overlap_height = A.rows/2;
    int overlap;

    int xoffset = 0;
    int yoffset = 0;
    int _rows, _cols; //Of Mat


    if (orientation == 1)
    {
        overlap = overlap_width;
        _rows = A.rows;
        _cols = A.cols + B.cols - overlap_width;
        xoffset = A.cols - overlap_width;
    }
    else if ( orientation == 2)
    {
        overlap = overlap_height;
        _rows = A.rows + B.rows - overlap_height;
        _cols = A.cols;
        yoffset = A.rows - overlap_height;
    }
  
    Mat no_graphcut(_rows, _cols, A.type() );
    A.copyTo(no_graphcut(Rect(0, 0, A.cols, A.rows)));
    B.copyTo(no_graphcut(Rect(xoffset, yoffset, B.cols, B.rows)));

    imshow("no", no_graphcut);

    int est_nodes;
    if (orientation == 1)      
        est_nodes = A.rows * overlap;
    else  
        est_nodes = A.cols * overlap;
    int est_edges = est_nodes * 4;

    cout << "overlap " << overlap << endl;
    cout << "nodos " << est_nodes << " edges " << est_edges << endl;
    GraphType g(est_nodes, est_edges);

    for(int i=0; i < est_nodes; i++) {
        g.add_node();
    }

    if (orientation == 1)
    {
  
        //If it's a square it doesnt matter if it checks rows or cols
        // Set the source/sink weights
        for(int y=0; y < A.rows; y++) {
            g.add_tweights(y*overlap_width + 0, INT_MAX, 0);
            g.add_tweights(y*overlap_width + overlap_width-1, 0, INT_MAX);
        }

        for(int y=0; y < A.rows; y++) {
            for(int x=0; x < overlap; x++) {
                int idx = y*overlap + x;

                Vec3b a0 = A.at<Vec3b>(y, xoffset + x);
                Vec3b b0 = B.at<Vec3b>(y, x);
                double cap0 = norm(a0, b0);

                    // Add right edge
                if(x+1 < overlap) {
                    Vec3b a1 = A.at<Vec3b>(y, xoffset + x + 1);
                    Vec3b b1 = B.at<Vec3b>(y, x + 1);

                     double cap1 = norm(a1, b1);

                    g.add_edge(idx, idx + 1, (int)(cap0 + cap1), (int)(cap0 + cap1));
                }

                // Add bottom edge
                if(y+1 < A.rows) {
                    Vec3b a2 = A.at<Vec3b>(y+1, xoffset + x);
                    Vec3b b2 = B.at<Vec3b>(y+1, x);

                    double cap2 = norm(a2, b2);

                    g.add_edge(idx, idx + overlap, (int)(cap0 + cap2), (int)(cap0 + cap2));
                }
            }
        }
    }

    else 
    {
        //Set the source/sink weights
        for(int x=0; x < A.cols; x++) {
            g.add_tweights(x*overlap + 0, INT_MAX, 0); // Add the Terminal nodes 
            g.add_tweights(x*overlap + overlap - 1, 0, INT_MAX);
        }
        for(int x=0; x < A.cols; x++) {
            for( int y=0; y < overlap; y++)  {
                int idy = x*overlap_height + y;

                Vec3b a0 = A.at<Vec3b>(y, xoffset + x);
                Vec3b b0 = B.at<Vec3b>(y, x);
                double cap0 = norm(a0, b0);

                
                 // Add bottom edge
                if(y+1 < overlap) {
                    Vec3b a1 = A.at<Vec3b>(yoffset + y + 1, x);
                    Vec3b b1 = B.at<Vec3b>(y + 1,x);
                    double cap1 = norm(a1, b1);
                    g.add_edge(idy, idy + 1, (int)(cap0 + cap1), (int)(cap0 + cap1));
                }

                // Add right edge
                if(x+1 < A.cols) {
                    Vec3b a2 = A.at<Vec3b>(yoffset + y, x+1);
                    Vec3b b2 = B.at<Vec3b>(y, x+1);
                    double cap2 = norm(a2, b2);
                    g.add_edge(idy, idy + overlap, (int)(cap0 + cap2), (int)(cap0 + cap2));
                }
            }
             
        }
    }
    

    int flow = g.maxflow();
    cout << "max flow: " << flow << endl;

    graphcut = no_graphcut.clone();
    graphcut_and_cutline = no_graphcut.clone();

    int idx, idy = 0;
    if (orientation == 1)
    {
        for(int y=0; y < A.rows; y++) {
            for(int x=0; x < overlap_width; x++) {
                if(g.what_segment(idx) == GraphType::SOURCE) {
                    graphcut.at<Vec3b>(y, xoffset + x) = A.at<Vec3b>(y, xoffset + x);
                }
                else {
                    graphcut.at<Vec3b>(y, xoffset + x) = B.at<Vec3b>(y, x);
                }
                graphcut_and_cutline.at<Vec3b>(y, xoffset + x) =  graphcut.at<Vec3b>(y, xoffset + x);

                // Draw the cut
                if(x+1 < overlap_width) {
                    if(g.what_segment(idx) != g.what_segment(idx+1)) {
                        graphcut_and_cutline.at<Vec3b>(y, xoffset + x) = Vec3b(0,0255,0);
                        graphcut_and_cutline.at<Vec3b>(y, xoffset + x + 1) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(y, xoffset + x - 1) = Vec3b(0,255,0);
                    }
                }

                // Draw the cut
                if(y > 0 && y+1 < A.rows) {
                    if(g.what_segment(idx) != g.what_segment(idx + overlap_width)) {
                        graphcut_and_cutline.at<Vec3b>(y-1, xoffset + x) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(y, xoffset + x) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(y+1, xoffset + x) = Vec3b(0,255,0);
                    }
                }
                idx++;
            }
        }
    }

    if (orientation == 2)
    {
        for(int x=0; x < A.cols; x++) {
            for( int y=0; y < overlap; y++)  {
                if(g.what_segment(idy) == GraphType::SOURCE) {
                    graphcut.at<Vec3b>(yoffset + y, x) = A.at<Vec3b>(yoffset + y, x);
                }
                else {
                    graphcut.at<Vec3b>(yoffset + y, x) = B.at<Vec3b>(y, x);
                }
                graphcut_and_cutline.at<Vec3b>(y, xoffset + x) =  graphcut.at<Vec3b>(y, xoffset + x);

                // Draw the cut
                if(y+1 < overlap) {
                    if(g.what_segment(idy) != g.what_segment(idy+1)) {
                        graphcut_and_cutline.at<Vec3b>(yoffset + y, x) = Vec3b(0,0255,0);
                        graphcut_and_cutline.at<Vec3b>(yoffset + y + 1, x) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(yoffset + y - 1, x) = Vec3b(0,255,0);
                    }
                }

                // Draw the cut
                //if(y > 0 && y+1 < A.rows) {
                if(x > 0 && x+1 < A.cols) {
                    if(g.what_segment(idy) != g.what_segment(idy + overlap_height)) {
                        graphcut_and_cutline.at<Vec3b>(yoffset + y, x-1) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(yoffset + y, x) = Vec3b(0,255,0);
                        graphcut_and_cutline.at<Vec3b>(yoffset + y, x+1) = Vec3b(0,255,0);
                    }
                }
                idy++;
            }
        }
    }

   // imwrite("graphcut.jpg", graphcut);
    //imwrite("graphcut_and_cut_line.jpg", graphcut_and_cutline);
    imshow("graphcut", graphcut);
    imshow("graphcut and cut line", graphcut_and_cutline);

    waitKey();
    return 0;*/
    cout << "djfadlfjnasd" << endl;
}
