cmake_minimum_required(VERSION 3.25)
project(ToDoApp)

set(CMAKE_CXX_STANDARD 17)

# Set the path to SFML
set(SFML_STATIC_LIBRARIES FALSE)
set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-2.5.1/lib/cmake/SFML")

# Add SFML include directory
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-2.5.1/include")

# Add SFML lib directory
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-2.5.1/lib")

# Find SFML
find_package(SFML 2.5.1 COMPONENTS system window graphics REQUIRED)

# Add source files
set(SOURCES
    gui_main.cpp
    task_manager.hpp
)

# Add executable
add_executable(ToDoApp ${SOURCES})

# Link SFML libraries (MinGW dynamic versions)
target_link_libraries(ToDoApp 
    sfml-graphics
    sfml-window
    sfml-system
)

# Copy SFML DLLs to output directory
if(WIN32)
    file(GLOB SFML_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-2.5.1/bin/*.dll")
    file(COPY ${SFML_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
