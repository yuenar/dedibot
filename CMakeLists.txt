cmake_minimum_required(VERSION 3.15)

# 设置CMAKE_TOOLCHAIN_FILE变量
# 如果没有定义CMAKE_TOOLCHAIN_FILE，则使用vcpkg的默认路径
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "D:/workspace/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
# 设置CMAKE_PREFIX_PATH变量
# 如果没有定义CMAKE_PREFIX_PATH，则使用vcpkg的默认路径


project(Dedibot LANGUAGES CXX)
set(PROJECT_NAME Dedibot)


if(WIN32)
    if(MSVC)
        # 或者更明确地指定源文件和执行字符集
        add_compile_options(
            $<$<C_COMPILER_ID:MSVC>:/source-charset:UTF-8>
            $<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>
            $<$<C_COMPILER_ID:MSVC>:/execution-charset:UTF-8>
            $<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>
        )
endif()
 add_compile_definitions(_USE_MATH_DEFINES)  # 修改这行
message("GUI system is WIN32 ${CMAKE_GENERATOR}")
endif()

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-v")

add_compile_options(-Wall)
add_compile_options("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
add_compile_options(-fPIC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(QtSingleApp)

find_package(Eigen3 CONFIG REQUIRED)
if(NOT TARGET Eigen3::Eigen)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
    if(NOT TARGET Eigen3::Eigen)
        message(FATAL_ERROR "Eigen3 not found. Please install Eigen3 or set the correct path.")
    endif()
endif()

find_package(OpenMesh REQUIRED)
if(NOT OpenMesh_FOUND)
    message(FATAL_ERROR "OpenMesh not found. Please install OpenMesh or set the correct path.")
    else()
    set(OPENMESH_DIR ${OpenMesh_DIR})
    message(STATUS "OpenMesh found at ${OPENMESH_DIR}")
    message(STATUS "OpenMesh include directory: ${OpenMesh_INCLUDE_DIRS}")
    message(STATUS "OpenMesh libraries: ${OpenMesh_LIBRARIES}")
endif()


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets OpenGL Network REQUIRED HINTS)

set(SOURCES
    main.cpp
    common.h
    resources.qrc
    AppIcon.rc
    lang_zh.ts
    Render/GLWidget.cpp
    Render/RenderEngine.cpp
    Data/MeshList.cpp
    Data/ModelData.cpp
    Data/ProjectData.cpp
    MeshG/ModelLayers.cpp
    MeshG/TriMeshGenerator.cpp
    FileRW/FileReaderWriter.cpp
    FileRW/SimpleTriangulation.cpp
    FileRW/readgcode.cpp
    MeshP/MeshProcess.cpp
    SupportG/SupportGenerator.cpp
    SupportG/CollisionDetector.cpp
    UI/EditTreeWidget.cpp
    UI/IconHelper.cpp
    UI/MainWindow.cpp
    UI/ModelListWidget.cpp
    UI/MoveModelWidget.cpp
    UI/RotateModelWidget.cpp
    UI/ScaleModelWidget.cpp
    UI/SupportParamWidget.cpp
    UI/SliceModelDialog.cpp
    UI/DediMessageBox.cpp
    UI/Uart.cpp
    Thread/FileRWThread.cpp
    Thread/SliceProcess.cpp
    Thread/SupportGThread.cpp
    Thread/ThreadPool.cpp
    qextserial/qextserialport.cpp
)
if(WIN32)
    list(APPEND SOURCES qextserial/qextserialport_win.cpp)
elseif(APPLE)
    list(APPEND SOURCES qextserial/qextserialport_mac.cpp)
else()
    list(APPEND SOURCES qextserial/qextserialport_unix.cpp)
endif(WIN32)

set(HEADERS
    common.h
    Render/GLWidget.h
    Render/RenderEngine.h
    Data/MeshList.h
    Data/ModelData.h
    Data/ProjectData.h
    MeshG/ModelLayers.h
    MeshG/TriMeshGenerator.h
    FileRW/FileReaderWriter.h
    FileRW/SimpleTriangulation.h
    FileRW/readgcode.h
    MeshP/MeshProcess.h
    SupportG/SupportGenerator.h
    SupportG/CollisionDetector.h
    UI/EditTreeWidget.h
    UI/IconHelper.h
    UI/MainWindow.h
    UI/ModelListWidget.h
    UI/MoveModelWidget.h
    UI/RotateModelWidget.h
    UI/ScaleModelWidget.h
    UI/SupportParamWidget.h
    UI/SliceModelDialog.h
    UI/DediMessageBox.h
    UI/Uart.h
    Thread/FileRWThread.h
    Thread/SliceProcess.h
    Thread/SupportGThread.h
    Thread/ThreadPool.h
    qextserial/qextserialport_global.h
    qextserial/qextserialport.h
)

set(FORMS
    UI/MainWindow.ui
    UI/ModelListWidget.ui
    UI/MoveModelWidget.ui
    UI/RotateModelWidget.ui
    UI/ScaleModelWidget.ui
    UI/SupportParamWidget.ui
    UI/SliceModelDialog.ui
    UI/Uart.ui
)

set(RESOURCES
    resources.qrc
)

qt5_add_resources(RESOURCES_RCC ${RESOURCES})
# 修复可执行目标配置
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES_RCC}
)

# 添加缺失的包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenMesh_INCLUDE_DIRS} # 添加OpenMesh头文件路径
    Render
    Data
    MeshG
    FileRW
    MeshP
    SupportG
    UI
    QtSingleApp
    qextserial
    Thread
)

# 修复OpenMesh链接配置
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:OpenMeshCored>
        $<$<CONFIG:Release>:OpenMeshCore>
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMeshCore)
endif()


target_link_libraries(${PROJECT_NAME}  
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Network
    SingleApplication::SingleApplication
    Eigen3::Eigen
    OpenMeshCore
    OpenMeshTools
)



