CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++20 -Wpedantic -fsanitize=address -g

all: p1

p1: main.cpp manager.o shell.o
	$(CXX) $(CXXFLAGS) main.cpp manager.o shell.o -o p1

manager.o: manager.cpp
	$(CXX) $(CXXFLAGS) -c manager.cpp -o manager.o

shell.o: shell.cpp
	$(CXX) $(CXXFLAGS) -c shell.cpp -o shell.o

clean:
	rm -f p1 manager.o shell.o
