
CC=g++

CPPUTEST_HOME = /usr/local/Cellar/cpputest/3.8
CPPFLAGS += -I$(CPPUTEST_HOME)/include/CppUTest
LDFLAGS = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt 

OBJs += test.o
OBJs += book.o
OBJs += testBook.o

readfile: readfile.o
	g++ readfile.o $(LDFLAGS) -o readfile
	./readfile

test: test.o book.o testBook.o
	g++ $(OBJs) $(LDFLAGS) -o test
	./test
clean:
	rm *.o test readfile
read_book.o: read_book.cpp
testBook.o: testBook.cpp
test.o: test.cpp
book.o: book.cpp book.hpp
readfile.o: readfile.cpp
