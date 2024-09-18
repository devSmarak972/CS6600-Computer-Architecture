# Compiler and flags
CXX = g++
CXXFLAGS = -Wall

# Target executable
TARGET = cache_sim

# Source files
SRCS = cache.cpp test1.cpp
# OBJS = $(SRCS:.cpp=.o)

# Default target
# all: $(TARGET)

# Rule to build the target executable
$(TARGET): cache.o
	$(CXX) $(CXXFLAGS) test1.cpp cache.o -o $(TARGET)

# Rule to build cache.o from cache.cpp
cache.o: cache.cpp
	$(CXX) $(CXXFLAGS) -c cache.cpp -o cache.o

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Rule to run the executable with specified arguments
run: $(TARGET)
	./$(TARGET) 1024 16 16 8192 4 sample_trace.txt