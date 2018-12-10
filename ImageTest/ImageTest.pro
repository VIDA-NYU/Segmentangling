TEMPLATE = app
CONFIG += console c++11 qt
CONFIG -= app_bundle

SOURCES += main.cpp

unix:!macx{
    LIBS += -lfreeimage -lfreeimageplus
}

win32-msvc*{
    INCLUDEPATH += ../FreeImage/Source/
    INCLUDEPATH += ../FreeImage/Wrapper/FreeImagePlus/
    LIBS += -L../FreeImage/Dist/x64/ -lFreeImage
    LIBS += -L../FreeImage/Wrapper/FreeImagePlus/dist/x64/ -lFreeImagePlus
}
