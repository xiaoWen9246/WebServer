CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g -I/usr/include/mysql

TARGET = test
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../test/test.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




