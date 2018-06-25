#ifndef __CONTOURTREE_H__
#define __CONTOURTREE_H__

#include "DisjointSets.hpp"
#include <iostream>
#include <Grid3D.hpp>
#include <chrono>
#include <MergeTree.hpp>
#include "ContourTreeData.hpp"
#include "SimplifyCT.hpp"
#include "Persistence.hpp"
#include "TriMesh.hpp"
#include "TopologicalFeatures.hpp"
#include "HyperVolume.hpp"
#include <fstream>
#include <iostream>
#include <cmath>

#include "ImageData.hpp"

void preProcessing(std::string dataName, int dimx, int dimy, int dimz);

#endif
