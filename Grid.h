#pragma once

#ifndef Grid_H
#define Grid_H

#include "poisson.h"

class Grid
{
public:
    Grid(int x , int y); //Constructor
    vector< vector<int> > grid;
    void fill();
    void mapGeneration();

private:
};

#endif