#include "networking.hpp"

/** Starts socket
 * @return 0 on success, -1 for errors
 */
int Sockets::startServer() {
    // defines hints to setup server
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // get address from given hints
    struct addrinfo *servinfo;
    if(getaddrinfo(Sockets::ip, std::to_string(Sockets::port).c_str(), &hints, &servinfo) != 0) {
        perror("Socket addrinfo error");
        return -1;
    }

    // setups socket
    Sockets::connection_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    // checks if the socket was created
    if(Sockets::connection_socket <= 0) {
        perror("Socket creation error");
        return -1;
    }

    // sets server socket to allow multiple connections in the same port
    int opt = 1;
    if(setsockopt(Sockets::connection_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        perror("Socket options error");
        return -1;
    }

    // binds socket to defined address
    if(bind(Sockets::connection_socket, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        perror("Socket bind error");
        return -1;
    }
    freeaddrinfo(servinfo);

    // Prepares for accepting connections
    if(listen(Sockets::connection_socket, Sockets::connection_number) < 0) {
        perror("Socket listen error");
        return -1;
    }

    printf("server listening\n");

    return 0;
}

/** starts a client socket
 * @return 0 on success, -1 for errors
 */
int Sockets::startClient() {
    // defines hints to setup client
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // get address from given hints
    struct addrinfo *clientinfo;
    if(getaddrinfo(Sockets::ip, std::to_string(Sockets::port).c_str(), &hints, &clientinfo) != 0) {
        perror("Socket addrinfo error");
        return -1;
    }

    // setups socket
    Sockets::connection_socket = socket(clientinfo->ai_family, clientinfo->ai_socktype, clientinfo->ai_protocol);

    // checks if the socket was created
    if(Sockets::connection_socket <= 0) {
        perror("Socket creation error");
        return -1; // error output
    }

    // connects client socket to server
    if(connect(Sockets::connection_socket, clientinfo->ai_addr, clientinfo->ai_addrlen) < 0) {
        perror("Socket connection error");
        return -1; // error output
    }
    freeaddrinfo(clientinfo);

    printf("client connected\n");

    return 0;
}

/** Client main listening function
 * @param com Message pointer to receive the incoming message
 * @param dropout_time struct in {seconds, milisseconds} that defines time until dropping the listening function, when no messages are received
 * @return 1 on success, 0 if server is down, -1 on local failure 
 */
int Sockets::loopClient(std::vector<std::string>* com) {
    // puts all sockets on list
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(Sockets::connection_socket, &fd_reads);

    // checks which sockets can be read
    ssize_t activity = select(connection_socket + 1, &fd_reads, NULL, NULL,  &(Sockets::dropout_time));

    // gets message if fd is set
    if (FD_ISSET(connection_socket, &fd_reads)) {
        char buffer[STD_SIZE];

        // reads message
        int message_size = recv(connection_socket, buffer, Sockets::STD_SIZE, 0);

        // read error
        if(message_size == -1) {
            perror("Socket read error");
            return -1; // error output
        }
        // socket closing signal
        else if (message_size == 0) {
            close(connection_socket);
            return 0; // close connection
        }
        // stores received message
        else {
            std::string new_string(buffer, message_size);
            (*com).emplace_back(new_string);
        }
    }

    return 1;
}

/** Server listening function
 * @param connection_list string array with all socket names
 * @param size size of the given arrays
 * @param server_com message pointer to receive incoming messages
 * @return 1 on success, 0 on receiving server message, -1 on critical failure
 */
int Sockets::loopServer(std::vector<std::string>* server_com) {
    // puts all sockets in a list and sets the maximum file descriptor
    int max_fd = Sockets::connection_socket;
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(connection_socket, &fd_reads);
    for(auto connected_socket : Sockets::sockets_list) {
        FD_SET(connected_socket, &fd_reads);
        if (connected_socket > max_fd) {
            max_fd = connected_socket;
        }
    }

    // checks which sockets can be read
    ssize_t activity = select(max_fd + 1, &fd_reads, NULL, NULL, &(Sockets::dropout_time));
    if (activity < 0) {
        perror("Socket select error");
        return -1;
    }

    // if server socket can be read, accepts new connections
    if(FD_ISSET(connection_socket, &fd_reads)) {
        int new_connection = accept(connection_socket, NULL, NULL);
        // accepts incoming connections
        if(new_connection == -1) {
            perror("Socket accept error");
        }
        else {
            Sockets::sockets_list.emplace_back(new_connection);
            printf("Stored new connection\n");
        }
    }

    // checks message received from each connection
    for (int i = 0; i < Sockets::sockets_list.size(); i++) {
        if (FD_ISSET(Sockets::sockets_list[i], &fd_reads)) {
            char buffer[STD_SIZE]; // message buffer

            // reads incoming message
            int message_size = recv(Sockets::sockets_list[i], buffer, Sockets::STD_SIZE, 0);

            // read error
            if (message_size == -1) {
                perror("Socket read error");
            }
            // socket closing signal
            else if (message_size == 0) {
                close(Sockets::sockets_list[i]);
                Sockets::sockets_list.erase(Sockets::sockets_list.begin() + i);
            }
            // stores received message
            else {
                std::string new_string(buffer, message_size);
                (*server_com).emplace_back(new_string);
            }
            
        }
    }

    return 1;
}

/** send message from client to server
 * @param socket socket to be sent the message
 * @param com Message to be sent
 * @return 0 on success, -1 for errors
 */
int Sockets::sendMessage(std::string msg) {
    if(send(Sockets::connection_socket, msg.c_str(), msg.size(), 0) < 0) {
        perror("Socket write error");
    }

    return 0;
}

/** send message from server to clients
 * @param socket socket to be sent the message
 * @param com Message to be sent
 * @return 0 on success, -1 for errors
 */
int Sockets::sendMessageAll(std::string msg) {
    for(auto fd : Sockets::sockets_list) {
        if(send(fd, msg.c_str(), msg.size(), 0) < 0) {
            perror("Socket write error");
        }
    }    

    return 0;
}

/** Closes socket
 * @param dis_socket socket to be disconnected
 */
void Sockets::closeSocket() {
    close(Sockets::connection_socket);
}
