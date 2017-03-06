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

void testDisjointSets() {
    int numElements = 128;
    int numInSameSet = 16;

    DisjointSets<int64_t> ds(numElements);
    int set1, set2;

    for (int k = 1; k < numInSameSet; k *= 2) {
        for (int j = 0; j + k < numElements; j += 2 * k) {
            set1 = ds.find(j);
            set2 = ds.find(j + k);
            ds.merge(set1, set2);
        }
    }

    for (int i = 0; i < numElements; i++) {
        std::cout << ds.find(i) << "*";
        if (i % numInSameSet == numInSameSet - 1)
            std::cout << "\n";
    }
    std::cout << "\n";
}

void testGrid() {
    qDebug() << "in test grid";
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    Grid3D grid(256,257,471);
    end = std::chrono::system_clock::now();
    qDebug() << "Test 1 - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    start = std::chrono::system_clock::now();
    grid.loadGrid("/home/harishd/Desktop/Projects/Fish/data/Fish_256/Fish_256.raw");
    MergeTree ct;
    ct.computeTree(&grid,JoinTree);
    end = std::chrono::system_clock::now();
    qDebug() << "Test 2 - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    ct.output("/home/harishd/Desktop/Projects/Fish/data/Fish_256/Fish_256", JoinTree);
}

void testSimplification2() {
    ContourTreeData ctdata;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    ctdata.loadBinFile("../data/Fish_256");
    end = std::chrono::system_clock::now();
    qDebug() << "Loading contour tree - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    start = std::chrono::system_clock::now();
    SimplifyCT sim;
    sim.setInput(&ctdata);
    Persistence per(ctdata);
    sim.simplify(&per);
    end = std::chrono::system_clock::now();
    qDebug() << "simplification - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    start = std::chrono::system_clock::now();
    std::vector<uint32_t> order = sim.order;
    SimplifyCT simo = SimplifyCT();
    simo.setInput(&ctdata);
    simo.simplify(order);
    end = std::chrono::system_clock::now();
    qDebug() << "simplification using order - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    qDebug() << "testing equivalence";
    for(int i = 0;i < order.size();i ++) {
        Branch b1 = sim.branches.at(order[i]);
        Branch b2 = simo.branches.at(order[i]);
        assert(b1.from != b2.from || b1.to != b2.to);
        assert(b1.parent == b2.parent);
        assert(b1.children == b2.children);
        assert(b1.arcs == b2.arcs);
    }
    qDebug() << "done!";

}

void testSimplification1() {
    ContourTreeData ctdata;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    ctdata.loadTxtFile("C:/Users/harishd/Desktop/Courses/Topology-2017/data/2d/assignment.rg");
    end = std::chrono::system_clock::now();
    qDebug() << "Loading contour tree - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    start = std::chrono::system_clock::now();
    SimplifyCT sim;
    sim.setInput(&ctdata);
    Persistence per(ctdata);
    sim.simplify(&per);
    end = std::chrono::system_clock::now();
    qDebug() << "simplification - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    for(int i = 0;i < sim.order.size();i ++) {
        Branch b1 = sim.branches.at(sim.order[i]);
        int v1 = b1.from;
        int v2 = b1.to;
        qDebug() << ctdata.nodeVerts[v1] << ctdata.nodeVerts[v2];
    }
    qDebug() << "done!";

}

void testPriorityQueue() {
    SimplifyCT sim;
    sim.queue.push(20);
    sim.queue.push(10);
    sim.queue.push(15);
    sim.queue.push(22);
    sim.queue.push(8);

    while(!sim.queue.empty()) {
        uint32_t top = sim.queue.top();
        sim.queue.pop();
        qDebug() << top;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    testGrid();
    testSimplification1();
//    testPriorityQueue();
    exit(0);
    return a.exec();
}
