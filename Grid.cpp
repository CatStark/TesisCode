#include "Grid.h"

Grid::Grid(int x, int y) //Constructor
{
	grid.resize( x , vector<int>(y));
}

void Grid::fill()
{
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid[0].size(); i++){
			double start = rand() % 255;
			grid[i][j] = start;
			cout << "[" <</*<< i << "][" << j << "]:" << */grid[i][j] << "]";
		}
		//cout << endl;
	}

	mapGeneration();
}

//For each cell we replace its value with the average value of all the cells around it
void Grid::mapGeneration()
{
	int tmpValue;
	int countOfValues; //How many data we have per cell to make the average
	double totalAvg = 0;
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid[0].size(); i++){
			countOfValues = 0;
			tmpValue = 0;
			//cout << "current i: " << i << " j: " << j << "   " << grid[i][j] << endl;
			if(i-1 >= 0){ //Check left side
				tmpValue += grid[i-1][j];
				countOfValues++;
				cout << grid[i-1][j] << " + " ;
				if(j+1 < grid[1].size()){ //Down
					tmpValue += grid[i-1][j+1];
					countOfValues++;
					cout << grid[i-1][j+1] <<" + " ;
				}
				if (j-1 < grid[1].size()){ //Up
					tmpValue += grid[i-1][j-1];
					countOfValues++;
					cout <<grid[i-1][j-1] <<" + "  ;
				}
			}

			if(i+1 < grid[0].size()){ //Check right side
				tmpValue += grid[i+1][j];
				countOfValues++;
				cout << grid[i+1][j] << " + " ;
				if(j+1 < grid[1].size()){ //Down
					tmpValue += grid[i+1][j+1];
					countOfValues++;
					cout << grid[i+1][j+1]  <<" + ";
				}
				if (j-1 < grid[1].size()){ //Up
					tmpValue += grid[i+1][j-1];
					countOfValues++;
					cout << grid[i+1][j-1] <<" + " ;
				}
			}

			if(j-1 >= 0){
				tmpValue += grid[i][j-1];
				countOfValues++;
				cout << grid[i][j-1]  <<" + ";
			}
			
			if(j+1 < grid[1].size()){
				tmpValue += grid[i][j+1];
				countOfValues++;
				cout << grid[i][j+1] <<" + " ;
			}
			//cout << endl;

			totalAvg += tmpValue/countOfValues;
			cout << " / " << countOfValues << " = " << tmpValue/countOfValues<< endl;
			grid[i][j] = tmpValue/countOfValues;
		}
	}

	cout << "new" << endl;
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid[0].size(); i++){
			cout << "[" << grid[i][j] << "]";
		}
		cout << endl;
	}

	//we run through each cell again and if it's less than the cell average the cell is type A or type B
	cout << "final" << endl;
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid[0].size(); i++){
			if (grid[i][j] < totalAvg / (grid[0].size() * grid[1].size()))
				grid[i][j] = 1;
			else
				grid[i][j] = 0;
			cout << "[" << grid[i][j] << "]";
		}
		cout << endl;
	}
}