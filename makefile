
CC=g++

CPPUTEST_HOME = /usr/local/Cellar/cpputest/3.8
CPPFLAGS += -I$(CPPUTEST_HOME)/include/CppUTest
LDFLAGS = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt 
CPPFLAGS += -std=c++0x

BUILD_DIR = target

OBJs +=test.o
OBJs +=book.o
OBJs +=testBook.o

test:$(OBJs) 
	g++ -o $(BUILD_DIR)/test $(OBJs)  $(LDFLAGS)
	mv *.o target/
	$(BUILD_DIR)/test

clean:
	rm $(BUILD_DIR)/* 

