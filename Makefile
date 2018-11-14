OUT = lsh cube cluster
OBJS = lsh_.o cube_.o cluster_.o LSH.o CUBE.o hash.o Cluster.o KMeans.o 
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) -std=c++11
LFLAGS = -Wall $(DEBUG)
HEADERS=$(wildcard *.h)
 
%.o: %.c $(HEADERS)
	$(CC) $(LFLAGS) -c $< -o $@

all: lsh cube cluster

cube: LSH.o CUBE.o hash.o cube_.o
	$(CC) $(LFLAGS) LSH.o CUBE.o hash.o cube_.o -o $@

lsh: LSH.o CUBE.o hash.o lsh_.o
	$(CC) $(LFLAGS) LSH.o CUBE.o hash.o lsh_.o -o $@

cluster: Cluster.o KMeans.o hash.o cluster_.o
	$(CC) $(LFLAGS) Cluster.o KMeans.o cluster_.o hash.o -o $@

clean: 
	rm -rf *.o $(OUT)

