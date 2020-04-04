.PHONY: all clean cleanall


C_SOURCES =  ../../../base64/base64.c
CXX_SOURCES = test.cpp ../utils.cpp

C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)

LIBS = ../../../nativefiledialog/build/lib/Release/x64/libnfd.a $(shell pkg-config --cflags --libs gtk+-3.0)

TARGET = test
INCLUDES = -I . -I .. -I ../../../ -I ../../../nativefiledialog/src/include/

all: $(C_OBJECTS) $(CXX_OBJECTS)
	g++ -lm $(INCLUDES) $(C_OBJECTS) $(CXX_OBJECTS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(C_OBJECTS) $(CXX_OBJECTS)

cleanall: clean
	rm -rf test

%.o:%.cpp
	g++ $(INCLUDES) -c $< -o $@

%.o:%.c
	gcc $(INCLUDES) -c $< -o $@
