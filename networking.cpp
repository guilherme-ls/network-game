#include "networking.hpp"

const char ip[]="127.0.0.1"; // for local host
//const ip[]="0.0.0.0"; // for allowing all incomming connection from internet
const int backlog=5; // maximum number of connection allowed

/** starts socket
 * @return 0 on success, -1 for errors
 */
int Sockets::open() {
    // creates an IPV4/IPV6 socket for sequential packets
    Sockets::connection_socket = socket(AF_UNSPEC, SOCK_SEQPACKET, 0);

    // checks if the socket was created
    if(Sockets::connection_socket <= 0) {
        perror("Socket creation error");
        return -1; // error output
    }

    return 0; // success output
}

/** Binds socket and prepares to receive connections
 * @param connection_number number of allowed socket connections
 * @return 0 on success, -1 for errors
 */
int Sockets::binding(unsigned int connection_number) {
    // sets server socket to allow multiple connections
    int opt = 1;
    if(setsockopt(Sockets::connection_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof opt) < 0) {
        perror("Socket options error");
        return -1;
    }

    // defines the socket address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_UNSPEC;
    address.sin_port = Sockets::port;

    // binds socket to defined port
    if(bind(Sockets::connection_socket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Socket bind error");
        return -1;
    }

    // Prepares for accepting connections
    if(listen(Sockets::connection_socket, connection_number) < 0) {
        perror("Socket listen error");
        return -1;
    }

    return 0;
}

/** Connects socket to given path
 * @param path path of the socket to be connected to
 * @return 0 on success, -1 for errors
 */
int Sockets::connecting(char* path) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    // connects client socket to server
    address.sin_family = AF_UNSPEC;
    address.sin_port = Sockets::port;
    connect(Sockets::connection_socket, (const struct sockaddr *) &address, sizeof(address));

    return 0;
}

/** Client main listening function
 * @param com Message pointer to receive the incoming message
 * @param dropout_time struct in {seconds, milisseconds} that defines time until dropping the listening function, when no messages are received
 * @return 1 on success, 0 if server is down, -1 on local failure 
 */
int Sockets::loopClient(char* com, struct timeval dropout_time) {
    // puts all sockets on list
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(Sockets::connection_socket, &fd_reads);

    // checks which sockets can be read
    ssize_t activity = select(connection_socket + 1, &fd_reads, NULL, NULL, &dropout_time);

    // gets message if fd is set
    if (FD_ISSET(connection_socket, &fd_reads)) {
        int temp;
        char buffer[STD_SIZE];

        // reads message
        if((temp = read(connection_socket, buffer, STD_SIZE)) == -1) {
            perror("Socket read error");
            return -1; // error output
        }
        else if (temp == 0) {
            close(connection_socket);
            return 0; // close connection
        }
        else {
            buffer[temp] = '\0';
            strcpy(com, buffer); // gets message
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
int Sockets::loopServer(std::string* connection_list, int size, std::vector<std::string>* server_com, struct timeval dropout_time) {
    // puts all sockets in a list and sets the maximum file descriptor
    int max_fd = Sockets::connection_socket;
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(connection_socket, &fd_reads);
    for (auto connected_socket : Sockets::sockets_list) {
        FD_SET(connected_socket, &fd_reads);
        if (connected_socket > max_fd) {
            max_fd = connected_socket;
        }
    }

    // checks which sockets can be read
    ssize_t activity = select(max_fd + 1, &fd_reads, NULL, NULL, &dropout_time);
    if ((activity < 0) && (errno != EINTR)) {
        printf("Socket select error");
        return -1;
    }

    // if server socket can be read, accepts new connections
    if (FD_ISSET(connection_socket, &fd_reads)) {
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
            int message_size = read(Sockets::sockets_list[i], buffer, STD_SIZE);

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
int sendMessage(int socket, char* msg) {
    if(write(socket, msg, strlen(msg)) < 0) {
        perror("Socket write error");
    }

    return 0;
}

/** Closes socket
 * @param dis_socket socket to be disconnected
 */
void closeSocket(int dis_socket) {
    struct sockaddr_un name;
    getpeername(dis_socket, (struct sockaddr *) &name, (socklen_t *) sizeof(name));
    close(dis_socket);
}
