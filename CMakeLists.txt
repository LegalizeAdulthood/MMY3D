cmake_minimum_required(VERSION 3.23)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MMY3D_PROJECT_NAME "MMY3D" CACHE "STRING" "Default project name for IDEs")
project("${MMY3D_PROJECT_NAME}" CXX)

set(CMAKE_MFC_FLAG 2)
add_executable(MMY3D WIN32
    AviFile.h
    MMY3D.h
    stdafx.h
    targetver.h
    AviFile.cpp
    stdafx.cpp
    MMY3D.cpp
    MMY3D.rc
    Resource.h
)
target_compile_definitions(MMY3D PUBLIC UNICODE _UNICODE)
target_precompile_headers(MMY3D PUBLIC stdafx.h)
target_link_libraries(MMY3D PUBLIC Vfw32.lib)
