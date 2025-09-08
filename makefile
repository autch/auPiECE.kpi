COMPILER=borland

include makefile.flags

TARGET = auPiECE.kpi

include makefile.files

$(TARGET): $(OBJS) $(DEF)
	$(MAKEDLL)

all: $(TARGET)

include makefile.dep

clean:
	rm -f *$(OBJEXT) muslib/*$(OBJEXT) $(WAVE)*$(OBJEXT) *.map *.tds
install:
	cp $(TARGET) $(DOC) $(KBMPLAY)

binarc: $(TARGET)
	zip -9j auPiECE_kpi-binary.zip $(TARGET) $(DOC) -x doc/compile.txt
srcarc:
	zip -9r auPiECE_kpi-source.zip * -x *.zip -x *$(OBJEXT)
arc: binarc srcarc
