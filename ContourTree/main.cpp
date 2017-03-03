#include <QCoreApplication>
#include <QDebug>

#include "DisjointSets.hpp"
#include <iostream>
#include <Grid3D.hpp>
#include <chrono>
#include <MergeTree.hpp>

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
    Grid3D grid(510,512,3685);
    end = std::chrono::system_clock::now();
    qDebug() << "Test 1 - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    start = std::chrono::system_clock::now();
    grid.loadGrid("/home/harishd/Desktop/Projects/Fish/data/Fish_512/Fish_512.raw");
    MergeTree ct;
    ct.computeTree(&grid,MergeTree::JoinTree);
    end = std::chrono::system_clock::now();
    qDebug() << "Test 2 - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    ct.output(MergeTree::JoinTree);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    testGrid();
    exit(0);
    return a.exec();
}
