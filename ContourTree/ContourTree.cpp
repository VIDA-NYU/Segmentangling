#include "ContourTree.hpp"
#include "MergeTree.hpp"
#include <vector>
#include <deque>
#include <cassert>
#include <QDebug>

namespace contourtree {

ContourTree::ContourTree() {
    noarcs = 0;
}

void ContourTree::setup(const MergeTree *tree) {
    this->tree = tree;
    nv = tree->data->getVertexCount();
    nodesJoin.resize(nv);
    nodesSplit.resize(nv);

    for(int64_t i = 0;i < nv;i ++) {
        // init
        nodesJoin[i].v = i;
        nodesSplit[i].v = i;

        // add join arcs
        int64_t to = i;
        int64_t from = tree->prev[to];
        if(from != -1) {
            nodesSplit[from].next.push_back(to);
            nodesSplit[to].prev.push_back(from);
        }

        // add split arcs
        to = tree->next[i];
        from = i;
        if(to != -1) {
            nodesJoin[from].next.push_back(to);
            nodesJoin[to].prev.push_back(from);
        }
    }
}

void ContourTree::computeCT() {
    std::deque<int64_t> q;
    for(int64_t v = 0;v < nv;v ++) {
        Node &jn = nodesJoin[v];
        Node &sn = nodesSplit[v];
        if(sn.next.size() + jn.prev.size() == 1) {
            q.push_back(v);
        }
    }

    while(q.size() > 0) {
        int xi = q.front();
        q.pop_front();
        Node &jn = nodesJoin[xi];
        Node &sn = nodesSplit[xi];
        if(sn.next.size() == 0 && sn.prev.size() == 0) {
            assert((jn.next.size() == 0 && jn.prev.size() == 0));
            continue;
        }

        if(sn.next.size() == 0) {
            if(sn.prev.size() > 1) {
                qDebug() << "Can this happen too???";
                assert(false);
            }
            int xj = sn.prev[0];
            remove(xi, nodesJoin);
            remove(xi, nodesSplit);

            int fr = xj;
            int to = xi;
            assert(fr < nv && to < nv);
            addArc(fr, to);
            if(nodesSplit[xj].next.size() + nodesJoin[xj].prev.size() == 1) {
                q.push_back(xj);
            }
        } else {
            if(jn.next.size() > 1) {
                qDebug() << "Can this happen too???";
                assert(false);
            }
            int xj = jn.next[0];
            remove(xi, nodesJoin);
            remove(xi, nodesSplit);

            int fr = xi;
            int to = xj;
            assert(fr < nv && to < nv);
            addArc(fr, to);

            if(nodesSplit[xj].next.size() + nodesJoin[xj].prev.size() == 1) {
                q.push_back(xj);
            }
        }
    }
}

void ContourTree::output(QString fileName) {

}

void ContourTree::remove(int64_t xi, std::vector<ContourTree::Node> &nodeArray) {
    Node &jn = nodeArray[xi];

    if(jn.prev.size() == 1 && jn.next.size() == 1) {
        int64_t p = jn.prev[0];
        int64_t n = jn.next[0];
        Node &pn = nodeArray[p];
        Node &nn = nodeArray[n];

        removeAndAdd(pn.next, xi, nn.v);
        removeAndAdd(nn.prev, xi, pn.v);
    } else if(jn.prev.size() == 0 && jn.next.size() == 1) {
        int n = jn.next[0];
        Node &nn = nodeArray[n];
        remove(nn.prev, xi);
    } else if(jn.prev.size() == 1 && jn.next.size() == 0) {
        int p = jn.prev[0];
        Node &pn = nodeArray[p];
        remove(pn.next, xi);
    } else {
        assert(false);
    }
}

void ContourTree::removeAndAdd(std::vector<int64_t> &arr, int64_t rem, int64_t add) {
    for(int i = 0;i < arr.size();i ++) {
        if(arr[i] == rem) {
            arr[i] = add;
            return;
        }
    }
    assert(false);
}

void ContourTree::remove(std::vector<int64_t> &arr, int64_t xi) {
    for(int i = 0;i < arr.size();i ++) {
        if(arr[i] == xi) {
            if(i != arr.size() - 1) {
                arr[i] = arr[arr.size() - 1];
            }
            arr.pop_back();
            return;
        }
    }
    assert(false);
}

void ContourTree::addArc(int64_t from, int64_t to) {
    noarcs ++;
}


}
