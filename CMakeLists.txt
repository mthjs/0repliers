cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

set(version 0.0.1)

project(
   0repliers
   VERSION "${version}"
   DESCRIPTION "Serving multiple REQ sockets with one ROUTER as the replier."
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
   0repliers SHARED
   src/0repliers.cpp
)

set_target_properties(
   0repliers PROPERTIES
   VERSION "${version}"
   PUBLIC_HEADER include/*.h
)
target_include_directories(0repliers PRIVATE include src)

install(
   TARGETS 0repliers
   LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
   PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

add_subdirectory(tests)
