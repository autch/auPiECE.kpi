add_library(muslib OBJECT
exp12.c
instdef.c
mus.c
musdef.h
musdefwin.h
muslib.h
seq.c
wavetable.c
)
target_include_directories(muslib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(wave)
