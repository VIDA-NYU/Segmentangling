#include "preprocessing.hpp"

using namespace contourtree;

void preProcessing(std::string dataName, int dimx, int dimy, int dimz) {
    // Assumes type to be unsigned char

    std::chrono::time_point<std::chrono::system_clock> start, end;
    Grid3D grid(dimx, dimy, dimz);

    std::string data = dataName;

    start = std::chrono::system_clock::now();
    grid.loadGrid(data + ".raw");
    MergeTree ct;
    contourtree::TreeType tree = TypeJoinTree;
    std::cout << "computing join tree" << std::endl;
    ct.computeTree(&grid, tree);
    end = std::chrono::system_clock::now();
    std::cout << "Time to compute contour tree: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
    ct.output(data, tree);


    std::cout << "creating hierarchical segmentation" << std::endl;
    // now simplify and store simplification hierarchy
    start = std::chrono::system_clock::now();
    ContourTreeData ctdata;
    ctdata.loadBinFile(data);

    SimplifyCT sim;
    sim.setInput(&ctdata);
    bool persistence = false;
    SimFunction *simFn;
    if (persistence) {
        simFn = new Persistence(ctdata);
    }
    else {
        simFn = new HyperVolume(ctdata, data + ".part.raw");
    }
    sim.simplify(simFn);
    end = std::chrono::system_clock::now();
    std::cout << "Time to simplify: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";

    sim.outputOrder(data);
    std::cout << "done" << std::endl;
}
