all: actrl

actrl: main.cpp
	g++ -Wall $(shell sdl-config --cflags --libs) main.cpp -o actrl
