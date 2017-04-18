QT += core
QT -= gui

CONFIG += c++11

TARGET = ContourTree
CONFIG -= app_bundle

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
    ContourTree.cpp

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
    test.hpp

# Unix configuration
unix:!macx{
    INCLUDEPATH += -I /usr/local/include/
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp
    LIBS += -lboost_thread -lboost_filesystem -lboost_program_options -lboost_system -lboost_iostreams
}

win32{
    CONFIG += console
    INCLUDEPATH += "$$(BOOST_PATH)"
    INCLUDEPATH += "$$(UniversalCRT_IncludePath)"

    LIBS += "-ladvapi32"
    LIBS += "-L$$(BOOST_LIB_PATH)"
    LIBS += "-L$$(UniversalCRT_LibraryPath_x64)"

#    QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#    QMAKE_LFLAGS_RELEASE += $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}
