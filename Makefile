CXX = g++
CXXFLAGS = -std=c++17 -Wall $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1) -I.
LIBS = $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1)

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = lexlp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean