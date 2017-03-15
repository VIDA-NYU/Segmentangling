#include "TriMesh.hpp"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSet>

namespace contourtree {

TriMesh::TriMesh()
{

}

int TriMesh::getMaxDegree() {
    return maxStar;
}

int TriMesh::getVertexCount() {
    return nv;
}

int TriMesh::getStar(int64_t v, QVector<int64_t> &star) {
    int ct = 0;
    foreach(uint32_t vv, vertices[v].adj) {
        star[ct] = vv;
        ct ++;
    }
    return ct;
}

bool TriMesh::lessThan(int64_t v1, int64_t v2) {
    if(fnVals[v1] < fnVals[v2]) {
        return true;
    } else if(fnVals[v1] == fnVals[v2]) {
        return (v1 < v2);
    }
    return false;
}

unsigned char TriMesh::getFunctionValue(int64_t v) {
    return this->fnVals[v];
}

void TriMesh::loadData(QString fileName)
{
    QFile ip(fileName);
    if(!ip.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "could not read file" << fileName;
    }
    QTextStream text(&ip);

    text.readLine();
    QStringList line = text.readLine().split(" ");
    nv = QString(line[0]).toInt();
    int nt = QString(line[1]).toInt();

    vertices.resize(nv);
    fnVals.resize(nv);
    for(int i = 0;i < nv;i ++) {
        line = text.readLine().split(" ");
        int fn = QString(line[3]).toInt();
        fnVals[i] = fn;
    }
    for(int i = 0;i < nt;i ++) {
        line = text.readLine().split(" ");
        int v1 = QString(line[1]).toInt();
        int v2 = QString(line[2]).toInt();
        int v3 = QString(line[3]).toInt();

        vertices[v1].adj.insert(v2);
        vertices[v1].adj.insert(v3);
        vertices[v2].adj.insert(v1);
        vertices[v2].adj.insert(v3);
        vertices[v3].adj.insert(v2);
        vertices[v3].adj.insert(v1);
    }

    maxStar = 0;
    for(int i = 0;i < nv;i ++) {
        maxStar = std::max(maxStar,vertices[i].adj.size());
    }
    qDebug() << maxStar;
}

}