
CC=g++

CPPUTEST_HOME = /usr/local/Cellar/cpputest/3.8
CPPFLAGS += -I$(CPPUTEST_HOME)/include/CppUTest
LDFLAGS = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt 
CPPFLAGS += -std=c++0x

BUILD_DIR = target

OBJs +=main.o
#OBJs +=BaseA.o
#OBJs +=BaseB.o
#OBJs +=Child.o

test:$(OBJs) 
	g++ -o test $(OBJs)  $(LDFLAGS)

clean:

