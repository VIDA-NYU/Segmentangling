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
#include "preprocessing.hpp"
using namespace contourtree;

int main(int argc, char *argv[])
{
    // need to call preprocessing using the data name
    std::string ipFolder= "/home/harishd/Desktop/Projects/Fish/data/straightening/OSF/Plagiotremus-tapinosoma";
    std::string filePrefix = "Plaagiotremus_tapinosoma_9.9um_2k__rec_Tra";
    std::string ext = "bmp";
    int stCt = 2;
    int enCt = 1798;

    std::string opFolder = "/home/harishd/Desktop/Projects/Fish/data/straightening/OSF/test";
    std::string opPrefix = "Plaagiotremus_tapinosoma";

    SamplingOutput op = ImageData::writeOutput(ipFolder,filePrefix,stCt,enCt,ext,opFolder,opPrefix,4,true);
    preProcessing(op.fileName,op.x,op.y,op.z);
    return 0;
}
