test: run
	./run

run: main.cc
	g++ -L/usr/local/lib -o run main.cc -std=c++11 -ltorrent-rasterbar -lpthread -lboost_system -Drrr=$$RANDOM

clean:
	rm run d.o
