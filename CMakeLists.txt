cmake_minimum_required(VERSION 3.0)

project(pylame)

set(CXX_STANDARD cxx_std_11)

find_package(PythonInterp REQUIRED 3.6)
find_package(PythonLibs)

if(PYTHONLIBS_FOUND)
    if(PYTHON_VERSION_MAJOR EQUAL 2)
        message(FATAL_ERROR "Must compile using python 3")
    endif()
    include_directories(${PYTHON_INCLUDE_DIRS})
endif()

find_library(LAME_FOUND mp3lame lame)
if(NOT LAME_FOUND)
    message(FATAL_ERROR "Require lame lab")
else()
    message(STATUS "Found Lame: ${LAME_FOUND}")
endif()

add_subdirectory("pybind11")

add_library(pylame MODULE encoder.cpp)
target_link_libraries(pylame PUBLIC mp3lame
                           PRIVATE pybind11::module)
set_target_properties(pylame PROPERTIES
                           PREFIX "${PYTHON_MODULE_PREFIX}"
                           SUFFIX "${PYTHON_MODULE_EXTENSION}")
