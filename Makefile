

all:
	g++ -o main *.cpp -g -lraylib -lGL -lm -lpthread -pthread -ldl -lrt -lX11 -pthread