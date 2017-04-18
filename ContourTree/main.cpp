#include <QCoreApplication>
#include <QDebug>

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
#include <cmath>

using namespace contourtree;

void preProcessing(QString rawFile, int dimx, int dimy, int dimz) {
    // Assumes type to be unsigned char

    std::chrono::time_point<std::chrono::system_clock> start, end;
    Grid3D grid(dimx,dimy,dimz);

    QString data = rawFile;
    if(rawFile.endsWith(".raw")) {
        data = rawFile.left(rawFile.length() - 4);
    }

    start = std::chrono::system_clock::now();
    grid.loadGrid(data + ".raw");
    MergeTree ct;
    contourtree::TreeType tree = TypeJoinTree;
    qDebug() << "computing join tree";
    ct.computeTree(&grid,tree);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to compute contour tree: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";
    ct.output(data, tree);


    qDebug() << "creating hierarchical segmentation";
    // now simplify and store simplification hierarchy
    start = std::chrono::system_clock::now();
    ContourTreeData ctdata;
    ctdata.loadBinFile(data);

    SimplifyCT sim;
    sim.setInput(&ctdata);
    bool persistence = false;
    SimFunction *simFn;
    if(persistence) {
        simFn = new Persistence(ctdata);
    } else {
        simFn = new HyperVolume(ctdata,data + ".part.raw");
    }
    sim.simplify(simFn);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to simplify: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    sim.outputOrder(data);
    qDebug() << "done";
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // need to call preprocessing using the data name

    exit(0);
    return a.exec();
}
