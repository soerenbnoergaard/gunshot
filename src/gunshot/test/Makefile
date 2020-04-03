.PHONY: all

SOURCES = test.cpp $(wildcard ../../../fftconvolver/*.cpp)
INCLUDES = -I . -I ../../../
TARGET = test

all:
	g++ -lm $(INCLUDES) -o $(TARGET) $(SOURCES)

