#ifndef TEST_HPP
#define TEST_HPP

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
    ct.computeTree(&grid,TypeJoinTree);
    end = std::chrono::system_clock::now();
    qDebug() << "Test 2 - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    ct.output("/home/harishd/Desktop/Projects/Fish/data/Fish_256/Fish_256", TypeJoinTree);
}

void testSimplification3() {
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

    sim.outputOrder("../data/Fish_256");

//    start = std::chrono::system_clock::now();
//    std::vector<uint32_t> order = sim.order;
//    SimplifyCT simo = SimplifyCT();
//    simo.setInput(&ctdata);
//    simo.simplify(order,-1,0.2f);
//    end = std::chrono::system_clock::now();
//    qDebug() << "simplification using order - Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

//    qDebug() << "counting remaining features";
//    int ct = 0;
//    for(int i = order.size() - 1;i >= 0;i --) {
//        if(simo.removed[order[i]]) {
//            break;
//        }
//        ct ++;
//    }
//    qDebug() << "Remaining features after simplification: " << ct << "of" << order.size();
//    qDebug() << "done!";
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

void testMergeTree() {
    TriMesh tri;
    tri.loadData("C:/Users/harishd/Desktop/Courses/Topology-2017/data/2d/assignment.off");
    MergeTree ct;
    ct.computeTree(&tri,TypeContourTree);
    qDebug() << "done";
    ct.output("C:/Users/harishd/Desktop/Courses/Topology-2017/data/2d/assignment",TypeContourTree);

    ContourTreeData ctdata;
    ctdata.loadBinFile("C:/Users/harishd/Desktop/Courses/Topology-2017/data/2d/assignment");
    qDebug() << "************** Merge Tree ********************";
    for(size_t i = 0;i < ctdata.noNodes;i ++) {
        qDebug() << ctdata.nodeVerts[i] << ctdata.fnVals[i] << (int)(ctdata.type[i]);
    }

    for(size_t i = 0;i < ctdata.noArcs;i ++) {
        qDebug() <<  ctdata.nodeVerts[ctdata.arcs[i].from] <<  ctdata.nodeVerts[ctdata.arcs[i].to];
    }

    SimplifyCT sim;
    sim.setInput(&ctdata);
    Persistence per(ctdata);
    sim.simplify(&per);
    sim.outputOrder("C:/Users/harishd/Desktop/Courses/Topology-2017/data/2d/assignment");
    qDebug() << "************** All branches ********************";;
    for(int i = 0;i < sim.order.size();i ++) {
        Branch b1 = sim.branches.at(sim.order[i]);
        int v1 = b1.from;
        int v2 = b1.to;
        qDebug() << ctdata.nodeVerts[v1] << ctdata.nodeVerts[v2];
    }


    SimplifyCT sim2;
    sim2.setInput(&ctdata);
    sim2.simplify(sim.order,3);
    qDebug() << "************** Remaining branches ********************";;
    for(int i = sim.order.size() - 1;i >= 0;i --) {
        if(sim2.removed[sim.order[i]]) {
            break;
        }
        Branch b1 = sim2.branches.at(sim.order[i]);
        int v1 = b1.from;
        int v2 = b1.to;
        qDebug() << ctdata.nodeVerts[v1] << ctdata.nodeVerts[v2];
    }

    qDebug() << "done!";
}

void toyProcessing() {
    // the actual raw file without the extension. the extensions will be added as and when needed.
    QString data = "../data/toy";

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    Grid3D grid(128,128,128);
    end = std::chrono::system_clock::now();

    start = std::chrono::system_clock::now();
    grid.loadGrid(data + ".raw");
    MergeTree ct;
    contourtree::TreeType tree = TypeJoinTree;
    ct.computeTree(&grid,tree);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to compute contour tree: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";
    ct.output(data, tree);


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

void testApi() {
    // the actual raw file without the extension. the extensions will be added as and when needed.
    QString data = "../data/Mass-Scan-Slice-Data-1-256-256-256-cropped";
    TopologicalFeatures tf;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    tf.loadData(data,true);
    std::vector<Feature> features = tf.getFeatures(-1,0.1f);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";

//    start = std::chrono::system_clock::now();
//    features = tf.getFeatures(-1,0.2f);
//    end = std::chrono::system_clock::now();
//    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
//    qDebug() << "no. of features:" << features.size() << "\n";

    qDebug() << "\n\n Partitioned features";
    start = std::chrono::system_clock::now();
    std::vector<Feature> features1 = tf.getFeatures(-1,0.1f);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";

    assert(features1.size() == features.size());
    for(int i = 0;i < features.size();i ++) {
        assert(features[i].to == features1[i].to);
    }

//    start = std::chrono::system_clock::now();
//    features = tf.getFeaturesPart(-1,0.2f);
//    end = std::chrono::system_clock::now();
//    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
//    qDebug() << "no. of features:" << features.size() << "\n";

}

void insert(QVector<int> &cx, QVector<int> &cy, QVector<int> &cz, QVector<int> &rad, int x,int y,int z, int r) {
    cx << x;
    cy << y;
    cz << z;
    rad << r;
}

void generateData(bool mini = false) {
    int dimx = 128;
    int dimy = 128;
    int dimz = 128;

    std::vector<uint8_t> volume;
    if(mini) {
        volume.resize(dimx * dimy * dimz, 255);
    } else {
        volume.resize(dimx * dimy * dimz, 0);
    }
    QVector<int> centerx, centery, centerz, rad;

//    insert(centerx,centery,centerz,30 ,30 , 30);
//    insert(centerx,centery,centerz,100,100, 30);
//    insert(centerx,centery,centerz,100,30 , 30);
//    insert(centerx,centery,centerz,30 ,100, 30);
//    insert(centerx,centery,centerz,30 ,30 , 100);
//    insert(centerx,centery,centerz,100,100, 100);
//    insert(centerx,centery,centerz,100,30 , 100);
//    insert(centerx,centery,centerz,30 ,100, 100);
    insert(centerx,centery,centerz,rad,128-40,128-40,30,20);
    insert(centerx,centery,centerz,rad,128-55,128-30,55,24);
    insert(centerx,centery,centerz,rad,128-90,128-80,128-60,30);

    // generate volume
    for(int i = 0;i < centerx.size();i ++) {
        int x = centerx[i];
        int y = centery[i];
        int z = centerz[i];
        int radius = rad[i];
        for(int xx = x - radius;xx < x + radius;xx ++) {
            for(int yy = y - radius;yy < y + radius;yy ++) {
                for(int zz = z - radius;zz < z + radius;zz ++) {
                    float dist = std::sqrt((x - xx) * (x - xx) + (y - yy) * (y - yy) + (z - zz) * (z - zz));
                    dist /= radius;
                    dist = (dist > 1)?1:dist;
                    int val = int((1 - dist) * 255);
                    int in = xx + yy * dimx + zz * dimx * dimy;
                    if(mini) {
                        val = 255 - val;
                    }
                    volume[in] = std::max(uint8_t(val),volume[in]);
                }
            }
        }
    }


    std::ofstream of;
    if(mini) {
        of.open("../data/toy-mini.raw",std::ios::binary);
    } else {
        of.open("../data/toy.raw",std::ios::binary);
    }
    of.write((char *)volume.data(),volume.size());
    of.close();
}

void testFeatures() {
    // the actual raw file without the extension. the extensions will be added as and when needed.
    QString data = "../data/ContourTree/Fish_256";
    TopologicalFeatures tf;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    tf.loadData(data);
    std::vector<Feature> features = tf.getFeatures(20,0);
//    std::vector<Feature> features = tf.getFeaturesPart(10,0);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";

    // read part file
    int dimx = 256;
    int dimy = 257;
    int dimz = 471;

    qDebug() << "reading part";
    QVector<uint32_t> part(dimx * dimy * dimz);
    std::ifstream ip((data + ".part.raw").toStdString(), std::ios::binary);
    ip.read((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();

    qDebug() << "mapping features to arcs";
    std::vector<uint32_t> arcMap(tf.ctdata.noArcs, 0);
    for(int i = 0;i < features.size();i ++) {
        for(int ano: features[i].arcs) {
            arcMap[ano] = (i + 1);
        }
    }

    qDebug() << "mapping voxels to features";
    for(int i = 0;i < part.size();i ++) {
        part[i] = arcMap[part[i]];
    }

    qDebug() << "writing features";
    std::ofstream op((data + ".test-features.raw").toStdString(), std::ios::binary);
    op.write((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();
}

void testConnectivity() {
//    QString data = "../data/ContourTree/Fish_256";
//    Grid3D grid(256,257,471);

    QString data = "../data/Cameroon/Cameroon_256";
    Grid3D grid(256,256,527);

    // read part file
    int dimx = 256;
    int dimy = 256;
    int dimz = 527;

    qDebug() << "reading part";
    std::vector<uint32_t> part(dimx * dimy * dimz);
    std::ifstream ip((data + ".part.raw").toStdString(), std::ios::binary);
    ip.read((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();


    qDebug() << "reading ct";
    ContourTreeData ctdata;
    ctdata.loadBinFile(data);

    uint32_t noArcs = ctdata.noArcs;
    std::vector<QSet<uint32_t> > arcs(noArcs);

    qDebug() << "arranging features";
    for(uint32_t i = 0;i < part.size();i ++) {
        uint32_t ano = part[i];
        arcs[ano] << i;
    }

    for(int i = 0;i < noArcs;i ++) {
        uint32_t from = ctdata.nodeVerts[ctdata.arcs[i].from];
        uint32_t to = ctdata.nodeVerts[ctdata.arcs[i].to];
        arcs[i] << from << to;
    }

    TopologicalFeatures tf;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    tf.loadData(data);
    std::vector<Feature> features = tf.getFeatures(15,0,0.3);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";


    DisjointSets<int32_t> dj;
    QVector<int64_t> star;
    QSet<uint32_t> comps;
    star.resize(grid.getMaxDegree());
    bool single = true;
    for(int i = 0;i < features.size();i ++) {
        qDebug() << "processing feature" << i;
        dj = DisjointSets<int32_t>(part.size());
        QSet<uint32_t> set;
        for(uint32_t ano: features[i].arcs) {
            foreach(uint32_t v, arcs[ano])
                set << v;
        }
        comps.clear();

        foreach(uint32_t v,set) {
            int ct = grid.getStar(v,star);

            for(int j = 0;j < ct;j ++) {
                if(set.contains(star[j])) {
                    dj.merge(v,star[j]);
                }
            }
        }
        foreach(uint32_t v,set) {
            comps << dj.find(v);
        }

        if(comps.size() != 1) {
            single = false;
            qDebug() << "Feature" << i << "has " << comps.size() << "components" << set.size();
        }
    }

    qDebug() << "all arcs have a single component:" << single;
}
void toyFeatures() {
    // the actual raw file without the extension. the extensions will be added as and when needed.
    QString data = "../data/toy";
    TopologicalFeatures tf;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    tf.loadData(data);
    std::vector<Feature> features = tf.getFeatures(2,0);
//    std::vector<Feature> features = tf.getFeaturesPart(10,0);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";

    // read part file
    int dimx = 128;
    int dimy = 128;
    int dimz = 128;

    qDebug() << "reading part";
    QVector<uint32_t> part(dimx * dimy * dimz);
    std::ifstream ip((data + ".part.raw").toStdString(), std::ios::binary);
    ip.read((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();

    qDebug() << "mapping features to arcs";
    std::vector<uint32_t> arcMap(tf.ctdata.noArcs, 0);
    for(int i = 0;i < features.size();i ++) {
        for(int ano: features[i].arcs) {
            arcMap[ano] = (i + 1);
        }
    }

    qDebug() << "mapping voxels to features";
    for(int i = 0;i < part.size();i ++) {
        part[i] = arcMap[part[i]];
    }

    qDebug() << "writing features";
    std::ofstream op((data + ".test-features.raw").toStdString(), std::ios::binary);
    op.write((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();
}

int oldMain(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    testGrid();
//    testSimplification3();
//    testPriorityQueue();
//    testMergeTree();
//    preProcessing();
//    testApi();
//    testFeatures();
//    testConnectivity();
    generateData();
    toyProcessing();
    toyFeatures();

    exit(0);
    return a.exec();
}


#endif // TEST_HPP
