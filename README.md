# Dedibot Project

Dedibot is a Qt-based application designed for 3D modeling and processing. This project utilizes various components to manage and manipulate 3D models, including rendering, file reading/writing, and user interface elements.

## Project Structure

The project consists of the following key components:

- **CMakeLists.txt**: Configuration file for CMake, defining build settings and dependencies.
- **main.cpp**: The entry point of the application, containing the main function.
- **common.h**: A common header file that may include global constants and function declarations.
- **resources.qrc**: Qt resource file defining resources used in the project.
- **AppIcon.rc**: Resource file containing the application icon definition.
- **lang_zh.ts**: Qt translation file containing Chinese translations.

### Source Files

- **Render**: Contains classes responsible for OpenGL rendering.
  - `GLWidget.cpp` / `GLWidget.h`: Implementation and header for the GLWidget class.
  - `RenderEngine.cpp` / `RenderEngine.h`: Implementation and header for the RenderEngine class.

- **Data**: Manages data related to models and projects.
  - `MeshList.cpp` / `MeshList.h`: Implementation and header for the MeshList class.
  - `ModelData.cpp` / `ModelData.h`: Implementation and header for the ModelData class.
  - `ProjectData.cpp` / `ProjectData.h`: Implementation and header for the ProjectData class.

- **MeshG**: Handles model layers and mesh generation.
  - `ModelLayers.cpp` / `ModelLayers.h`: Implementation and header for the ModelLayers class.
  - `TriMeshGenerator.cpp` / `TriMeshGenerator.h`: Implementation and header for the TriMeshGenerator class.

- **FileRW**: Responsible for file reading and writing operations.
  - `FileReaderWriter.cpp` / `FileReaderWriter.h`: Implementation and header for the FileReaderWriter class.
  - `SimpleTriangulation.cpp` / `SimpleTriangulation.h`: Implementation and header for the SimpleTriangulation class.
  - `readgcode.cpp` / `readgcode.h`: Implementation and header for G-code reading functionality.

- **MeshP**: Processes mesh operations.
  - `MeshProcess.cpp` / `MeshProcess.h`: Implementation and header for the MeshProcess class.

- **SupportG**: Generates support structures and handles collision detection.
  - `SupportGenerator.cpp` / `SupportGenerator.h`: Implementation and header for the SupportGenerator class.
  - `CollisionDetector.cpp` / `CollisionDetector.h`: Implementation and header for the CollisionDetector class.

- **UI**: Contains user interface components.
  - Various `.cpp`, `.h`, and `.ui` files for different UI elements, including main window, model list, and dialogs.

- **QtSingleApp**: Manages single instance application functionality.
  - Various `.cpp` and `.h` files for handling local peer and file locking.

- **Thread**: Implements threading for file reading and processing tasks.
  - Various `.cpp` and `.h` files for handling threads and thread pools.

- **qextserial**: Handles serial communication.
  - Various `.cpp` and `.h` files for serial port management.

### Resources

- **resource.h**: Header file for resource definitions.
- **README.md**: Documentation for the project.

## Build Instructions

To build the project, ensure you have CMake and Qt installed. Navigate to the project directory and run the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.