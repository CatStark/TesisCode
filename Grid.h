#pragma once

#ifndef Grid_H
#define Grid_H

#include "Patch.h"

class Grid
{
public:
    Grid(int x , int y); //Constructor
    std::vector< vector<int> > grid;
    void fill();
    void mapGeneration();

private:
};

#endif