CXX         = g++
CXXFLAGS    = -Wall -O3
SOURCES     = main.cpp tspsolver.cpp bruteforcetspsolver.cpp
OBJECTS     = $(patsubst %.cpp,%.o,$(SOURCES))
TARGET      = tspsolver

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
