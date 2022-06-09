# Makefile

TARGET=pa1.out
CXX=gcc
CXXFLAGS=-w
OBJECTS=main.o

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
main.o: main.c
	$(CXX) $(CXXFLAGS) -c $^
clean:
	rm $(OBJECTS) $(TARGET)
