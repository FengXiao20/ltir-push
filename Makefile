# ltir006 —— Linux / Termux，C++17（core 的 rational.hpp 用 std::string_view）
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
INC       = -I core/include

ltir006: src/ltir006.cpp src/value.hpp core/src/rational.cpp
	$(CXX) $(CXXFLAGS) $(INC) -o $@ src/ltir006.cpp core/src/rational.cpp

clean:
	rm -f ltir006

.PHONY: clean
