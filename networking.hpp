#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

class Sockets {
    public:

        const int STD_SIZE = 128;

        const int port = 4277;

        std::vector<int> sockets_list;

        int openServer();

        int openClient(std::string name);

        int loopClient(char* com, struct timeval dropout_time);

        int loopServer(std::string* connection_list, int size, std::vector<std::string>* server_com, struct timeval dropout_time);

        int sendMessage(int socket, std::string msg);

        void closeSocket(int dis_socket);

    private:
        int connection_socket;

        int open();

        int binding(unsigned int connection_number);

        int connecting(char* path);

        int listenning(char* com, struct timeval dropout_time);
};