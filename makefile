include makefile.flags

TARGET = auPiECE.kpi

all: $(TARGET)

include makefile.dep makefile.files

$(TARGET): $(OBJS) $(DEF)
	dllwrap $(DBGFLAGS) $(ENVFLAGS) $(DLLFLAGS) --dllname=$@ --def=$(DEF) $(OBJS) -lstdc++
clean:
	rm -f *.o muslib/*.o muslib/wave/*.o
install:
	cp $(TARGET) $(KBMPLAY)Plugins/
binarc:
	zip -9j auPiECE_kpi-binary.zip auPiECE.kpi doc/readme.txt
srcarc: clean
	zip -9r auPiECE_kpi-source.zip * -x *.zip
