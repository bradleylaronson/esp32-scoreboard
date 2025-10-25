CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I../../shared

all: test-simulator

test-simulator: main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test: test-simulator
	./test-simulator

clean:
	rm -f test-simulator

