TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle

SOURCES += main.cpp

INCLUDEPATH += ../FreeImage/Source/
INCLUDEPATH += ../FreeImage/Wrapper/FreeImagePlus/

unix:!macx{

}

win32-msvc*{
    LIBS += -L../FreeImage/Dist/x64/ -lFreeImage
    LIBS += -L../FreeImage/Wrapper/FreeImagePlus/dist/x64/ -lFreeImagePlus
}
