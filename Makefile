CXX=mpicxx
CXXFLAGS=-O3 -march=native
LDFLAGS=-O2 -std=c++17
all: main main2

main: main.cpp prng.h
	$(CXX) $(LDFLAGS) -o $@ $<

main2: main2.cpp prng.h
	$(CXX) $(LDFLAGS) -o $@ $<

.PHONY: all clean

clean:
	$(RM) main main2 