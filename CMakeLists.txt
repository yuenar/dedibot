cmake_minimum_required(VERSION 3.16)

# 设置vcpkg工具链
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/zhangyu1/workspaces/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(Dedibot VERSION 1.0.0 LANGUAGES CXX)

# 设置CMAKE_PREFIX_PATH到vcpkg安装路径
if(WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "C:/Users/zhangyu1/workspaces/vcpkg/installed/x64-windows" CACHE STRING "")
endif()





if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    include(ProcessorCount)
    ProcessorCount(N)
    if(N GREATER 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    message(STATUS "Debug build enabled")
endif()

# Windows特定配置
if(WIN32)
    if(MSVC)
        # 设置UTF-8编码
        add_compile_options(
            $<$<C_COMPILER_ID:MSVC>:/source-charset:UTF-8>
            $<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>
            $<$<C_COMPILER_ID:MSVC>:/execution-charset:UTF-8>
            $<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>
        )
endif()

if(VCPKG_TOOLCHAIN)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif()

add_compile_definitions(_USE_MATH_DEFINES)
message("GUI system is WIN32 ${CMAKE_GENERATOR}")
endif()

# CMake策略设置
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

# C++标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(NOT WIN32)
    add_compile_options(-Wall)
    add_compile_options("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
    add_compile_options(-fPIC)
endif()

# Qt相关设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 添加QtSingleApp子目录
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(QtSingleApp)

# 查找Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets OpenGL Network REQUIRED)

# 查找Eigen3（可选）
find_package(Eigen3 CONFIG QUIET)
if(NOT Eigen3_FOUND)
    find_package(Eigen3 QUIET)
    if(NOT Eigen3_FOUND)
        message(WARNING "Eigen3 not found. Trying to find manually...")
        find_path(EIGEN3_INCLUDE_DIR Eigen/Core
            PATHS
            "C:/Users/zhangyu1/workspaces/vcpkg/installed/x64-windows/include"
            "C:/vcpkg/installed/x64-windows/include"
            "${CMAKE_PREFIX_PATH}/include"
            /usr/local/include/eigen3
            /usr/include/eigen3)
        if(EIGEN3_INCLUDE_DIR)
            message(STATUS "Found Eigen3 headers at: ${EIGEN3_INCLUDE_DIR}")
            set(EIGEN3_FOUND TRUE)
        else()
            message(FATAL_ERROR "Eigen3 not found. Please install via vcpkg: vcpkg install eigen3:x64-windows")
        endif()
    endif()
endif()

# 查找OpenMesh（可选）
find_package(OpenMesh CONFIG REQUIRED)
if(NOT OpenMesh_FOUND)
    message(FATAL_ERROR "OpenMesh not found. Please install OpenMesh or set the correct path.")
    else()
    set(OPENMESH_DIR ${OpenMesh_DIR})
    message(STATUS "OpenMesh found at ${OPENMESH_DIR}")
    message(STATUS "OpenMesh include directory: ${OpenMesh_INCLUDE_DIRS}")
    message(STATUS "OpenMesh libraries: ${OpenMesh_LIBRARIES}")
endif()

# if(NOT OpenMesh_FOUND)
#     message(WARNING "OpenMesh not found via CONFIG. Trying alternative method...")
#     find_path(OPENMESH_INCLUDE_DIR OpenMesh/Core/Mesh/TriMesh.hh
#               PATHS
#               "C:/Users/zhangyu1/workspaces/vcpkg/installed/x64-windows/include"
#               "C:/vcpkg/installed/x64-windows/include"
#               "${CMAKE_PREFIX_PATH}/include")

#     find_library(OPENMESH_CORE_LIBRARY
#                  NAMES OpenMeshCore
#                  PATHS
#                  "C:/Users/zhangyu1/workspaces/vcpkg/installed/x64-windows/lib"
#                  "C:/vcpkg/installed/x64-windows/lib"
#                  "${CMAKE_PREFIX_PATH}/lib")

#     if(OPENMESH_INCLUDE_DIR AND OPENMESH_CORE_LIBRARY)
#         message(STATUS "Found OpenMesh manually:")
#         message(STATUS "  Include: ${OPENMESH_INCLUDE_DIR}")
#         target_include_directories(${PROJECT_NAME} PRIVATE ${OPENMESH_INCLUDE_DIRS})
#         message(STATUS "  Library: ${OPENMESH_CORE_LIBRARY}")
#         set(OPENMESH_FOUND TRUE)
#     else()
#         message(WARNING "OpenMesh not found. Please install via vcpkg: vcpkg install openmesh:x64-windows")
#         set(OPENMESH_FOUND FALSE)
#     endif()
# endif()


# 查找OpenMesh DLL文件
find_file(OPENMESH_CORE_DLL
    NAMES OpenMeshCore.dll OpenMeshCored.dll
    PATHS
    ${OPENMESH_ROOT_DIR}/bin
    ${OPENMESH_ROOT_DIR}/build/Build/bin
    PATH_SUFFIXES Release Debug
)

find_file(OPENMESH_TOOLS_DLL
    NAMES OpenMeshTools.dll OpenMeshToolsd.dll
    PATHS
    ${OPENMESH_ROOT_DIR}/bin
    ${OPENMESH_ROOT_DIR}/build/Build/bin
    PATH_SUFFIXES Release Debug
)

# # 复制DLL到输出目录
# if(OPENMESH_CORE_DLL)
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${OPENMESH_CORE_DLL}
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>
#     )
# endif()

# if(OPENMESH_TOOLS_DLL)
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${OPENMESH_TOOLS_DLL}
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>
#     )
# endif()



# 源文件
set(SOURCES
    main.cpp
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

# 平台特定源文件
if(WIN32)
    list(APPEND SOURCES qextserial/qextserialport_win.cpp)
elseif(APPLE)
    list(APPEND SOURCES qextserial/qextserialport_unix.cpp)
else()
    list(APPEND SOURCES qextserial/qextserialport_unix.cpp)
endif()

# 头文件
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

# UI文件
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

# 资源文件
set(RESOURCES
    resources.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# Windows RC文件
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE AppIcon.rc)
endif()

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "C:/Users/zhangyu1/workspaces/vcpkg/installed/x64-windows/include"
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

# 添加Eigen3包含目录
if(EIGEN3_FOUND)
    if(TARGET Eigen3::Eigen)
        target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
    elseif(EIGEN3_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR})
    endif()
endif()

# # 添加OpenMesh包含目录和库
# if(OPENMESH_FOUND)
#     if(TARGET OpenMeshCore)
#         target_link_libraries(${PROJECT_NAME} PRIVATE OpenMeshCore OpenMeshTools)
#     elseif(OPENMESH_INCLUDE_DIR)
#         target_include_directories(${PROJECT_NAME} PRIVATE ${OPENMESH_INCLUDE_DIR})
#         if(OPENMESH_CORE_LIBRARY)
#             target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENMESH_CORE_LIBRARY})
#         endif()
#     endif()
# endif()

# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Network
    SingleApplication::SingleApplication
    OpenMeshCore
    OpenMeshTools
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 调试信息
message(STATUS "=== Configuration Summary ===")
message(STATUS "Qt version: ${QT_VERSION}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "Eigen3 found: ${EIGEN3_FOUND}")
message(STATUS "OpenMesh found: ${OPENMESH_FOUND}")
if(EIGEN3_INCLUDE_DIR)
    message(STATUS "Eigen3 include: ${EIGEN3_INCLUDE_DIR}")
endif()
if(OPENMESH_INCLUDE_DIR)
    message(STATUS "OpenMesh include: ${OPENMESH_INCLUDE_DIR}")
endif()
message(STATUS "==============================")
