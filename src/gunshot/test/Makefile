.PHONY: all clean cleanall


C_SOURCES =  ../../../base64/base64.c
CXX_SOURCES = test.cpp ../utils.cpp $(wildcard ../../../fftconvolver/*.cpp)

C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)

TARGET = test
INCLUDES = -I . -I ../../../ -I..

all: $(C_OBJECTS) $(CXX_OBJECTS)
	g++ -lm $(INCLUDES) $(C_OBJECTS) $(CXX_OBJECTS) -o $(TARGET)

clean:
	rm -f $(C_OBJECTS) $(CXX_OBJECTS)

cleanall: clean
	rm -rf test

%.o:%.cpp
	g++ $(INCLUDES) -c $< -o $@

%.o:%.c
	gcc $(INCLUDES) -c $< -o $@
