run: ./build/main
	./build/main

./build/main: main.cpp | build
	g++ -std=c++11 -o $@ $< -I../../include

build:
	mkdir -p build
