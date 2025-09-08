.SUFFIXES: .c .o

.c.o:
	gcc -c -mno-cygwin $<

OBJS=i_BD909.o i_HC909.o i_SDGATE.o   i_TOMM1.o i_CYMBD.o \
     i_HO909.o i_TOMH1.o i_HANDCLAP.o i_SD909.o i_TOML1.o

wave: $(OBJS)

i_BD909.o: i_BD909.c ../musdef.h
i_CYMBD.o: i_CYMBD.c ../musdef.h
i_HANDCLAP.o: i_HANDCLAP.c ../musdef.h
i_HC909.o: i_HC909.c ../musdef.h
i_HO909.o: i_HO909.c ../musdef.h
i_SD909.o: i_SD909.c ../musdef.h
i_SDGATE.o: i_SDGATE.c ../musdef.h
i_TOMH1.o: i_TOMH1.c ../musdef.h
i_TOML1.o: i_TOML1.c ../musdef.h
i_TOMM1.o: i_TOMM1.c ../musdef.h
