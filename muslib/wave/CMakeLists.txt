add_library(muslib_wave OBJECT
i_BD909.c
i_CYMBD.c
i_HANDCLAP.c
i_HC909.c
i_HO909.c
i_SD909.c
i_SDGATE.c
i_TOMH1.c
i_TOML1.c
i_TOMM1.c
)
target_include_directories(muslib_wave PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
