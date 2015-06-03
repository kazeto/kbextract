TARGET = extractor
CXX = g++

SOURCE = $(shell ls src/*.cpp)
OBJS = $(SOURCE:.cpp=.o)

OPTS = -g -std=c++11

all: $(OBJS)
	$(CXX) $(OPTS) $(OBJS) -o $(TARGET)

.cpp.o:
	$(CXX) $(OPTS) -c -o $(<:.cpp=.o) $<

clean:
	rm -f $(TARGET)
	rm -f src/*.o
