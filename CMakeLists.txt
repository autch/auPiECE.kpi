cmake_minimum_required(VERSION 3.20)
project(aupiece.kpi LANGUAGES C CXX)

add_subdirectory(muslib)

add_library(aupiece SHARED
context.cpp
context.h
dllexports.c
kmp_pi.h
version.h
winlayer.cpp
winlayer.h

auPiECE.def
)
target_link_libraries(aupiece muslib muslib_wave)

set_target_properties(aupiece PROPERTIES SUFFIX ".kpi")
