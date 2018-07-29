bin = main
interfaces = $(wildcard *.cppm)
src = $(wildcard *.cpp)

.PHONY: clean all

modules = $(patsubst %.cppm, %.pcm, $(interfaces))
objects = $(patsubst %.cpp, %.o, $(src))
CXX = ./c++ -std=c++17 -fmodules-ts -fprebuilt-module-path=.

all: $(bin)

$(bin): $(objects)
	$(CXX) $^ -o $@

# if public dependency
main.pcm: hello.pcm

$(objects): $(modules)
# actually on per-module basis
#hello.o: hello.pcm
#main.o: main.pcm hello.pcm

clean:
	rm -f $(bin) $(objects) $(modules)

.SUFFIXES: .o .cpp .pcm .cppm
.cpp.o:
	$(CXX) -fmodule-file=$*.pcm -c $< -o $@
.cppm.pcm:
	$(CXX) --precompile $< -o $@
