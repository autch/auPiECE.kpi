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

binarc:
	if [ -e Release/*.kpi ]; then cp release/*.kpi .; fi
	zip -9j auPiECE_kpi-binary.zip $(TARGET) $(DOC)
srcarc:
	zip -9 auPiECE_kpi-source.zip * doc/* muslib/*.? muslib/wave/*.c release/*.kpi -x *.zip *$(OBJEXT) *.tds *.ncb
arc: binarc srcarc
