CONFIG += c++11

TARGET = ContourTree
CONFIG -= app_bundle
CONFIG -= qt

TEMPLATE = app

SOURCES += main.cpp \
    MergeTree.cpp \
    Grid3D.cpp \
    SimplifyCT.cpp \
    ContourTreeData.cpp \
    Persistence.cpp \
    TriMesh.cpp \
    TopologicalFeatures.cpp \
    HyperVolume.cpp \
    ContourTree.cpp \
    ImageData.cpp \
    preprocessing.cpp \
    Logger.cpp

HEADERS += \
    DisjointSets.hpp \
    MergeTree.hpp \
    ScalarFunction.hpp \
    Grid3D.hpp \
    SimplifyCT.hpp \
    ContourTreeData.hpp \
    constants.h \
    SimFunction.hpp \
    Persistence.hpp \
    TriMesh.hpp \
    TopologicalFeatures.hpp \
    HyperVolume.hpp \
    ContourTree.hpp \
    test.hpp \
    ImageData.hpp \
    stb_image.h \
    preprocessing.hpp \
    Logger.hpp

# Unix configuration
unix:!macx{
    INCLUDEPATH += -I /usr/local/include/
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp
    LIBS += -lfreeimage -lfreeimageplus
}

win32{
    CONFIG += console
    INCLUDEPATH += "$$(BOOST_PATH)"
    INCLUDEPATH += ../FreeImage/Source/
    INCLUDEPATH += ../FreeImage/Wrapper/FreeImagePlus/

    LIBS += "-ladvapi32"
    LIBS += -L../FreeImage/Dist/x64/ -lFreeImage
    LIBS += -L../FreeImage/Wrapper/FreeImagePlus/dist/x64/ -lFreeImagePlus

#    QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#    QMAKE_LFLAGS_RELEASE += $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}
