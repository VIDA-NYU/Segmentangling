#ifndef SCALARFUNCTION
#define SCALARFUNCTION

#include <QVector>
#include <stdint.h>

class ScalarFunction {

public:
    virtual int getMaxDegree() = 0;
    virtual int getVertexCount() = 0;
    virtual QVector<int64_t> getStar(int64_t v) = 0;
    virtual bool lessThan(int64_t v1, int64_t v2) = 0;
    virtual unsigned char getFunctionValue(int64_t v) = 0;
};

#endif // SCALARFUNCTION

