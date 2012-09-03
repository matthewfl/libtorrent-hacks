run: main.cc
	g++ -ggdb -O0 -Wall -std=c++11 -w -o d.o -c main.cc
	g++ -o run d.o -ltorrent-rasterbar -lboost_system -lpthread

test: run
	./run

clean:
	rm run d.o
