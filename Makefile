# always compile with highest -std=c++XX
build:
	g++ -std=c++23 -o lith lith.cpp
clean:
	rm lith
