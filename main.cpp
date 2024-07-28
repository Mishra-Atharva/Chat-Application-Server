#include <iostream>
#include <arpa/inet.h>
#include <optional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <thread>
#include <cstring>
#include <ostream> 

//Manger Class
class Manager
{
private:
    std::mutex send_and_recv_mtx;

public:
    void send_message(int client, std::string message)
    {
        std::unique_lock<std::mutex> send_lock(send_and_recv_mtx);

        ssize_t send_len = send(client, message.c_str(), message.length() + 1, 0);

        send_lock.unlock();
    }

    void recv_message(int client) 
    {
        std::unique_lock<std::mutex> recv_lock(send_and_recv_mtx);

        char* message = new char[2048];

        ssize_t recv_len = recv(client, message, 2048 - 1, 0);
        if (recv_len < 1)
        {
            delete[] message;
        }

        recv_lock.unlock();

        std::cout << message;
    }
};

//Setup the socket
int setup()
{

    //Setting up the socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    //Error Handling
    if (socket_fd == -1)
    {
        std::cout << "[!] Error: " << std::strerror(errno) << std::endl;

        return 1;
    }

    //Creating socket address
    sockaddr_in socket_address;
    
    //IPV4
    socket_address.sin_family = AF_INET;

    //PORT 
    socket_address.sin_port = htons(3306);

    //SERVER IP
    socket_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    //Connecting to the server
    int result = connect(socket_fd, (sockaddr*)&socket_address, sizeof(socket_address));
    
    //Error Handling
    if (result == -1)
    {
        std::cout << "[!] Error: " << std::strerror(errno) << std::endl;

        return 1;
    }

    //SUCCESS
    return 0;
}

void start(int client, Manager& manage)
{
    std::string message;
    manage.recv_message(client);

    //User enters nickname
    std::getline(std::cin >> std::ws, message);
    manage.send_message(client, message);


    while(message != "/quit")
    {

    }
}

// Main
int main()
{
    //Class
    Manager manage;

    //Setting up the connection
    int server_fd = setup();

    //Error Handling
    if (server_fd == 1)
    {
        std::cout << "[!] Unable to connect to client..." << std::endl;

        return 1;
    }

    //SUCCESS
    std::cout << "[*] Connected to the server...!" << std::endl;

    //Creating thread
    std::thread start_chat(start, server_fd, std::ref(manage));
    start_chat.join();

    return 0;
}
