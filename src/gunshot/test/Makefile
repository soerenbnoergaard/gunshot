.PHONY: all clean cleanall

SOURCES = test.cpp ../utils.cpp $(wildcard ../../../fftconvolver/*.cpp)
INCLUDES = -I . -I ../../../ -I..
TARGET = test

all:
	g++ -lm $(INCLUDES) -o $(TARGET) $(SOURCES)

clean:

cleanall: clean
	rm -rf test
