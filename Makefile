

all:
	g++ -o main *.cpp -g -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -pthread