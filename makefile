COMPILER=borland

include makefile.flags

TARGET = auPiECE.kpi

all: $(TARGET)

include makefile.dep makefile.files

$(TARGET): $(OBJS) $(DEF)
	$(MAKEDLL)
clean:
	rm -f *$(OBJEXT) muslib/*$(OBJEXT) muslib/wave/*$(OBJEXT)
install:
	cp $(TARGET) $(KBMPLAY)
binarc:
	zip -9j auPiECE_kpi-binary.zip auPiECE.kpi doc/readme.txt
srcarc: clean
	zip -9r auPiECE_kpi-source.zip * -x *.zip
