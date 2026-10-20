CXX      = /opt/homebrew/opt/llvm@17/bin/clang++
CXXFLAGS = -std=c++26 -O3 -Wall -Wextra

reference: reference.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

solution: solution.cpp
	$(CXX) $(CXXFLAGS) -O3 -mcpu=native $< -o $@

solution-debug: solution.cpp
	$(CXX) $(CXXFLAGS) -O3 -g -mcpu=native $< -o $@

clean:
	rm -f reference solution solution-debug
