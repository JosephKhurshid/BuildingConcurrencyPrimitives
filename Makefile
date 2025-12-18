all: counter 

counter: counter.o alllocks.o
	g++ counter.o alllocks.o -pthread -O3 -std=c++20 -g -o counter

counter.o: counter.cpp alllocks.h
	g++ -c counter.cpp -pthread -O3 -std=c++20 -g -o counter.o

alllocks.o: alllocks.cpp alllocks.h
	g++ -c alllocks.cpp -pthread -O3 -std=c++20 -g -o alllocks.o

clean:
	rm -f counter.o alllocks.o counter
