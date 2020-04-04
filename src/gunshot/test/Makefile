.PHONY: all clean cleanall

SOURCES = test.cpp ../utils.cpp $(wildcard ../../../fftconvolver/*.cpp) ../../../base64/base64.c
INCLUDES = -I . -I ../../../ -I..
TARGET = test

all:
	g++ -lm $(INCLUDES) -o $(TARGET) $(SOURCES)

clean:

cleanall: clean
	rm -rf test
