all: trash.m.cpp trash_node.cpp
	g++ -std=c++11 -lboost_filesystem -lboost_system trash.m.cpp trash_node.cpp -o trash

clean:
	rm trash
