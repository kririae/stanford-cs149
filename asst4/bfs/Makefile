all: default grade

default: main.cpp bfs.cpp
	clang++ -I../ -std=c++20 -fopenmp -O3 -g -o bfs main.cpp bfs.cpp ../common/graph.cpp ref_bfs.o -ltbb
grade: grade.cpp bfs.cpp
	clang++ -I../ -std=c++20 -fopenmp -O3 -g -o bfs_grader grade.cpp bfs.cpp ../common/graph.cpp ref_bfs.o -ltbb
clean:
	rm -rf bfs_grader bfs  *~ *.*~
