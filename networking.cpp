#include "networking.hpp"

void Sockets::clearSocket() {
    connection_socket = -1;
    inbound_messages.clear();
    outbound_messages.clear();
    sockets_list.clear();
    max_fd = 0;
    halt_loop = false;
    FD_ZERO(&fd_reads);
}

/** Starts socket
 * @return 0 on success, -1 for errors
 */
int Sockets::startServer() {
    // defines hints to setup server
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    halt_loop = false;

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
    halt_loop = false;

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

    printf("Client connected\n");

    return 0;
}

void Sockets::receiveMessage(int nsock) {
    printf("started receive message thread\n");
    char buffer[STD_SIZE]; // message buffer

    // reads incoming message
    int message_size = recv(nsock, buffer, Sockets::STD_SIZE, 0);

    // read error
    if (message_size == -1) {
        perror("Socket read error");
    }
    // socket closing signal
    else if (message_size == 0) {
        printf("Closed socket connection\n");
        if(close(nsock) < 0) {
            perror("Socket close error");
        }
        // removes server and ends connection
        if(nsock == connection_socket) {
            mutex_halt_loop.lock();
            halt_loop = true;
            mutex_halt_loop.unlock();
            connection_socket = -1;
            printf("Server disconnected\n");
        }
        // removes disconnected socket from socket list (if server)
        else {
            mutex_alter_socket_list.lock();
            auto iter = std::find(Sockets::sockets_list.begin(), Sockets::sockets_list.end(), nsock);
            *iter = -1;
            mutex_alter_socket_list.unlock();
            printf("A client disconnected\n");
        }
    }
    // stores received message
    else {
        std::string new_string(buffer, message_size);
        mutex_alter_inbound_messages.lock();
        inbound_messages.emplace_back(make_pair(new_string, nsock));
        mutex_alter_inbound_messages.unlock();
    }
}

std::string serialize(std::vector<std::array<std::array<int, 4>, 4>>* board) {
    std::string output = "brd ";
    int fractions = board->size();
    output += std::to_string(fractions) + " ";
    for(int i = 0; i < fractions; i++) {
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                output += std::to_string((*board)[i][j][k]) + " ";
            }
        }
    }

    return output;
}

void Sockets::acceptConnections() {
    printf("Started accept thread\n");
    int new_connection = accept(connection_socket, NULL, NULL);
    // accepts incoming connections
    if(new_connection == -1) {
        perror("Socket accept error");
    }
    // updates socket list with new connection
    else {
        int i = 0;
        mutex_alter_socket_list.lock();
        for(; i < sockets_list.size(); i++) {
            if(sockets_list[i] == -1) {
                sockets_list[i] = new_connection;
                break;
            }
        }
        if(i == sockets_list.size())
            sockets_list.emplace_back(new_connection);
        mutex_alter_socket_list.unlock();

        // send messages to setup the game
        std::thread message_thread1(&Sockets::sendMessage, this, new_connection, "num " + std::to_string(i + 1) + " " + std::to_string(control->player_turn));
        std::thread message_thread2(&Sockets::sendMessage, this, new_connection, serialize(&(control->positions)));
        message_thread1.detach();
        message_thread2.detach();
        printf("Stored new connection\n");
    }
}

/** Client main listening function
 * @param com Message pointer to receive the incoming message
 * @param dropout_time struct in {seconds, milisseconds} that defines time until dropping the listening function, when no messages are received
 * @return 1 on success, 0 if server is down, -1 on local failure 
 */
int Sockets::receiveLoopClient() {
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(Sockets::connection_socket, &fd_reads);

    // checks if socket can be read
    ssize_t activity = select(connection_socket + 1, &fd_reads, NULL, NULL,  &(Sockets::dropout_time));
    if(activity < 0) {
        perror("Socket select error");
        return -1;
    }

    // gets message if fd is set
    if (FD_ISSET(connection_socket, &fd_reads)) {
        receiveMessage(connection_socket);
        /*std::thread receive_thread(&Sockets::receiveMessage, this, connection_socket);
        receive_thread.detach();*/
    }

    return 1;
}

/** Server listening function
 * @param connection_list string array with all socket names
 * @param size size of the given arrays
 * @param server_com message pointer to receive incoming messages
 * @return 1 on success, 0 on receiving server message, -1 on critical failure
 */
int Sockets::receiveLoopServer() {
    // puts all sockets in a list and sets the maximum file descriptor
    int max_fd = Sockets::connection_socket;
    fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(connection_socket, &fd_reads);
    for(auto connected_socket : Sockets::sockets_list) {
        if(connected_socket == -1)
            continue;

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

    // checks message received from each connection
    std::vector<std::thread*> message_threads;
    for (int i = 0; i < Sockets::sockets_list.size(); i++) {
        if(sockets_list[i] == -1)
            continue;

        mutex_alter_socket_list.lock();
        int nsock = Sockets::sockets_list[i];
        mutex_alter_socket_list.unlock();

        if(FD_ISSET(nsock, &fd_reads)) {
            message_threads.emplace_back(new std::thread(&Sockets::receiveMessage, this, nsock));
        }
    }

    // if server socket can be read, accepts new connections
    if(FD_ISSET(connection_socket, &fd_reads)) {
        acceptConnections();
    }

    // joins previous threads to ensure no overlapping read operation next cycle
    for(auto threads : message_threads) {
        threads->join();
    }

    return 1;
}

void Sockets::sendLoopClient() {
    mutex_alter_outbound_messages.lock();
    int size = outbound_messages.size();
    mutex_alter_outbound_messages.unlock();

    // sends all stored messages
    std::vector<std::thread*> message_threads;
    for(int i = 0; i < size; i++) {
        std::pair<std::string, int> msg;
        mutex_alter_outbound_messages.lock();
        auto iter = outbound_messages.cbegin();
        msg = *iter;
        outbound_messages.erase(iter);
        mutex_alter_outbound_messages.unlock();

        // sends message
        message_threads.emplace_back(new std::thread(&Sockets::sendMessage, this, connection_socket, msg.first));
    }

    // joins previous threads to ensure no overlapping write operation next cycle
    for(auto send_thread : message_threads) {
        send_thread->join();
    }
}

void Sockets::sendLoopServer() {
    mutex_alter_outbound_messages.lock();
    int size = outbound_messages.size();
    mutex_alter_outbound_messages.unlock();

    // sends all stored messages
    std::vector<std::thread*> message_threads;
    for(int i = 0; i < size; i++){
        std::pair<std::string, int> msg;
        mutex_alter_outbound_messages.lock();
        auto iter = outbound_messages.cbegin();
        msg = *iter;
        outbound_messages.erase(iter);
        mutex_alter_outbound_messages.unlock();

        // sends message
        mutex_alter_socket_list.lock();
        for(int i = 0; i < sockets_list.size(); i++) {
            if(sockets_list[i] == -1)
                continue;

            if(sockets_list[i] != msg.second) {
                message_threads.emplace_back(new std::thread(&Sockets::sendMessage, this, sockets_list[i], msg.first));
            }
        }
        mutex_alter_socket_list.unlock();
    }
    
    // joins previous threads to ensure no overlapping write operation next cycle
    for(auto send_thread : message_threads) {
        send_thread->join();
    }
}

/** send message to specified socket
 * @param fd file descriptor of the socket to be sent the message
 * @param msg message to be sent
 * @return 0 on success, -1 for errors
 */
int Sockets::sendMessage(int fd, std::string msg) {
    printf("Started send message thread\n");
    if(send(fd, msg.c_str(), msg.size(), 0) < 0) {
        perror("Socket write error");
        
        // puts message back in the queue if it fails to send
        mutex_alter_outbound_messages.lock();
        outbound_messages.emplace_back(make_pair(msg, fd));
        mutex_alter_outbound_messages.unlock();
        
        return -1;
    }
    return 0;
}

void Sockets::realClientLoop() {
     while(1) {
        // receives messages
        receiveLoopClient();
        
        // deals with received messages
        mutex_alter_inbound_messages.lock();
        int size = inbound_messages.size();
        mutex_alter_inbound_messages.unlock();

        for(int i = 0; i < size; i++) {
            std::pair<std::string, int> msg;
            mutex_alter_inbound_messages.lock();
            auto iter = inbound_messages.cbegin();
            msg = *iter;
            inbound_messages.erase(iter);
            mutex_alter_inbound_messages.unlock();

            // deal with msg
            control->mutex_alter_inbound_messages.lock();
            control->inbound_messages.emplace_back(msg.first);
            control->mutex_alter_inbound_messages.unlock();
        }

        // gets messages to be sent
        control->mutex_alter_outbound_messages.lock();
        size = control->outbound_messages.size();
        control->mutex_alter_outbound_messages.unlock();

        for(int i = 0; i < size; i++) {
            std::string msg;
            control->mutex_alter_outbound_messages.lock();
            auto iter = control->outbound_messages.cbegin();
            msg = *iter;
            control->outbound_messages.erase(iter);
            control->mutex_alter_outbound_messages.unlock();

            mutex_alter_outbound_messages.lock();
            outbound_messages.emplace_back(make_pair(msg, connection_socket));
            mutex_alter_outbound_messages.unlock();
        }

        // terminates client loop thread
        mutex_halt_loop.lock();
        if(halt_loop) {
            mutex_halt_loop.unlock();
            return;
        }
        mutex_halt_loop.unlock();

        // sends messages
        sendLoopClient();
    }
}

void Sockets::realServerLoop() {
    while(1) {
        // receives messages
        receiveLoopServer();
        
        // deals with received messages
        mutex_alter_inbound_messages.lock();
        int size = inbound_messages.size();
        mutex_alter_inbound_messages.unlock();

        for(int i = 0; i < size; i++) {
            std::pair<std::string, int> msg;
            mutex_alter_inbound_messages.lock();
            auto iter = inbound_messages.cbegin();
            msg = *iter;
            inbound_messages.erase(iter);
            mutex_alter_inbound_messages.unlock();
            
            // deal with msg
            control->mutex_alter_inbound_messages.lock();
            control->inbound_messages.emplace_back(msg.first);
            control->mutex_alter_inbound_messages.unlock();

            mutex_alter_outbound_messages.lock();
            outbound_messages.emplace_back(msg);
            mutex_alter_outbound_messages.unlock();
        }

        // gets messages to be sent
        control->mutex_alter_outbound_messages.lock();
        size = control->outbound_messages.size();
        control->mutex_alter_outbound_messages.unlock();

        for(int i = 0; i < size; i++) {
            std::string msg;
            control->mutex_alter_outbound_messages.lock();
            auto iter = control->outbound_messages.cbegin();
            msg = *iter;
            control->outbound_messages.erase(iter);
            control->mutex_alter_outbound_messages.unlock();

            mutex_alter_outbound_messages.lock();
            outbound_messages.emplace_back(make_pair(msg, connection_socket));
            mutex_alter_outbound_messages.unlock();
        }

        // terminates server loop thread
        mutex_halt_loop.lock();
        if(halt_loop) {
            mutex_halt_loop.unlock();
            return;
        }
        mutex_halt_loop.unlock();

        // sends messages
        sendLoopServer();
    }
}

/** Closes socket connection
 */
void Sockets::endConnection(std::thread* network_thread) {
    mutex_halt_loop.lock();
    halt_loop = true;
    mutex_halt_loop.unlock();
    network_thread->join();

    // closes all client sockets
    for(auto sock : sockets_list) {
        if(sock != -1)
            if(close(sock) < 0)
                perror("Socket close connected client error");
    }

    // closes socket if not already closed
    if(connection_socket > 0) {
        printf("Closed connection socket\n");
        if(close(Sockets::connection_socket) < 0)
            perror("Socket close error");
    }
}