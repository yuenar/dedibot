#-------------------------------------------------
#
# Project created by QtCreator 2015-10-21T15:18:56
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dedibot
TEMPLATE = app

win32 {
    DEFINES += _USE_MATH_DEFINES
    INCLUDEPATH += $$(OPENMESH)/include \
                    Eigen
    CONFIG(debug,   debug|release){LIBS += -L$$(OPENMESH)/x86/lib -lOpenMeshCored}
    CONFIG(release, debug|release){LIBS += -L$$(OPENMESH)/x86/lib -lOpenMeshCore }

    SOURCES += qextserial/qextserialport_win.cpp
}
unix {
     SOURCES += qextserial/qextserialport_unix.cpp
}

macx {
    LIBS += -stdlib=libc++
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
    QMAKE_LFLAGS += -mmacosx-version-min=10.9
    INCLUDEPATH += /usr/local/include \
            /usr/local/include/eigen3
    LIBS += -L/usr/local/lib \
        -lOpenMeshCore
}

SOURCES += main.cpp \
    Render/GLWidget.cpp \
    Render/RenderEngine.cpp \
    Data/MeshList.cpp \
    Data/ModelData.cpp \
    Data/ProjectData.cpp \
    MeshG/ModelLayers.cpp \
    MeshG/TriMeshGenerator.cpp \
    FileRW/FileReaderWriter.cpp \
    FileRW/SimpleTriangulation.cpp \
    MeshP/MeshProcess.cpp \
    SupportG/SupportGenerator.cpp \
    UI/EditTreeWidget.cpp \
    UI/IconHelper.cpp \
    UI/MainWindow.cpp \
    UI/ModelListWidget.cpp \
    UI/MoveModelWidget.cpp \
    UI/RotateModelWidget.cpp \
    UI/ScaleModelWidget.cpp \
    UI/SupportParamWidget.cpp \
    UI/SliceModelDialog.cpp \
    SupportG/CollisionDetector.cpp \
    QtSingleApp/qtlocalpeer.cpp \
    QtSingleApp/qtsingleapplication.cpp \
    QtSingleApp/qtsinglecoreapplication.cpp \
    UI/DediMessageBox.cpp \
    QtSingleApp/qtlockedfile.cpp \
    QtSingleApp/qtlockedfile_unix.cpp \
    QtSingleApp/qtlockedfile_win.cpp \
    Thread/FileRWThread.cpp \
    Thread/SliceProcess.cpp \
    Thread/SupportGThread.cpp \
    Thread/ThreadPool.cpp \
    qextserial/qextserialport.cpp \
    UI/Uart.cpp \
    FileRW/readgcode.cpp

HEADERS  += \
    common.h \
    Render/GLWidget.h \
    Render/RenderEngine.h \
    Data/MeshList.h \
    Data/ModelData.h \
    Data/ProjectData.h \
    MeshG/ModelLayers.h \
    MeshG/TriMeshGenerator.h \
    FileRW/FileReaderWriter.h \
    FileRW/SimpleTriangulation.h \
    MeshP/MeshProcess.h \
    SupportG/SupportGenerator.h \
    UI/EditTreeWidget.h \
    UI/IconHelper.h \
    UI/MainWindow.h \
    UI/ModelListWidget.h \
    UI/MoveModelWidget.h \
    UI/RotateModelWidget.h \
    UI/ScaleModelWidget.h \
    UI/SupportParamWidget.h \
    common.h \
    UI/SliceModelDialog.h \
    SupportG/CollisionDetector.h \
    QtSingleApp/qtlocalpeer.h \
    QtSingleApp/qtlockedfile.h \
    QtSingleApp/qtsingleapplication.h \
    QtSingleApp/qtsinglecoreapplication.h \
    QtSingleApp/QtLockedFile \
    QtSingleApp/QtSingleApplication \
    UI/DediMessageBox.h \
    Thread/FileRWThread.h \
    Thread/SliceProcess.h \
    Thread/SupportGThread.h \
    Thread/ThreadPool.h \
    resource.h \
    qextserial/qextserialport_global.h \
    qextserial/qextserialport.h \
    UI/Uart.h \
    FileRW/readgcode.h

FORMS    += \
    UI/MainWindow.ui \
    UI/ModelListWidget.ui \
    UI/MoveModelWidget.ui \
    UI/RotateModelWidget.ui \
    UI/ScaleModelWidget.ui \
    UI/SupportParamWidget.ui \
    UI/SliceModelDialog.ui \
    UI/Uart.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS +=\
lang_zh.ts

RC_FILE += \
AppIcon.rc
