
CC=g++

CPPUTEST_HOME = /usr/local/Cellar/cpputest/3.8
CPPFLAGS += -I$(CPPUTEST_HOME)/include/CppUTest
LDFLAGS = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt 
CPPFLAGS += -std=c++0x

BUILD_DIR = target

OBJs +=$(BUILD_DIR)/test.o
OBJs +=$(BUILD_DIR)/book.o
OBJs +=$(BUILD_DIR)/testBook.o

$(BUILD_DIR)/%.o:%.cpp
	g++ -c -o $@ $<

#readfile: readfile.o
#	g++ readfile.o $(LDFLAGS) -o readfile

$(BUILD_DIR)/test:$(OBJs) 
	g++ -o $@  $(LDFLAGS)
#	./test
clean:
	rm $(BUILD_DIR)/* 

#read_book.o: read_book.cpp
#target/testBook.o: testBook.cpp
#target/test.o: test.cpp
#target/book.o: book.cpp book.hpp
#readfile.o: readfile.cpp
