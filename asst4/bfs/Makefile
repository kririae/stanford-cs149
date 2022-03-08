all: default grade

default: main.cpp bfs.cpp
	clang++ -I../ -std=c++20 -fopenmp -O3 -g -o bfs main.cpp bfs.cpp ../common/graph.cpp ref_bfs.o -fsanitize=address
grade: grade.cpp bfs.cpp
	clang++ -I../ -std=c++20 -fopenmp -O3 -g -o bfs_grader grade.cpp bfs.cpp ../common/graph.cpp ref_bfs.o -fsanitize=address
clean:
	rm -rf bfs_grader bfs  *~ *.*~
