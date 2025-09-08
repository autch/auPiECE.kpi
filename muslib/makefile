.PHONY: all wave muslib
.SUFFIXES: .c .o
.c.o:
	gcc -c -mno-cygwin $<

OBJS=exp12.o instdef.o mus.o seq.o wavetable.o

all: muslib wave

muslib: $(OBJS)

wave:
	make -C wave -f makefile

exp12.o: exp12.c
instdef.o: instdef.c musdef.h
mus.o: mus.c musdef.h musdefwin.h
seq.o: seq.c musdef.h
wavetable.o: wavetable.c musdef.h

