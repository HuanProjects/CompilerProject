# Build the executable named 'parser' from the object files
parser: parser.o TreeNode.o pugixml.o
	g++ -g -o parser parser.o TreeNode.o pugixml.o

# Compile the main program file
parser.o: parser.cpp TreeNode.h
	g++ -g -c parser.cpp -I.

# Compile the pugixml library file
pugixml.o: pugixml.cpp pugixml.hpp
	g++ -g -c pugixml.cpp -I.

# Compile the TreeNode implementation file
TreeNode.o: TreeNode.cpp TreeNode.h
	g++ -g -c TreeNode.cpp

# Clean the built files
clean:
	rm -f parser *.o