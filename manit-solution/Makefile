CXX      = /opt/homebrew/opt/llvm@17/bin/clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra

reference: reference.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

solution: solution.cpp
	$(CXX) $(CXXFLAGS) -O3 -mcpu=native $< -o $@

clean:
	rm -f reference solution
