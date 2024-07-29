#include <iostream>
#include <arpa/inet.h>
#include <optional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <thread>
#include <cstring>
#include <ostream> 
#include <optional>

//Manger Class
class Manager
{
private:
    std::mutex send_and_recv_mtx;

public:
    void send_message(int server, std::string message)
    {
        std::unique_lock<std::mutex> send_lock(send_and_recv_mtx);

        ssize_t send_len = send(server, message.c_str(), message.length() + 1, 0);

        send_lock.unlock();
    }

    auto recv_message(int server) -> std::optional<std::string>
    {
        std::unique_lock<std::mutex> recv_lock(send_and_recv_mtx);

        char* message = new char[2048];

        ssize_t recv_len = recv(server, message, 2048 - 1, 0);
        if (recv_len < 1)
        {
            delete[] message;
            return {};
        }

        recv_lock.unlock();

        return std::string(message);
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
    return socket_fd;
}

void recv_chat(int server, Manager& manage)
{
    auto recv_msg = manage.recv_message(server);
    while(true)
    {
        if (recv_msg.has_value())
        {
            std::string converted_message = recv_msg.value();
            std::cout << converted_message;
        }
    }
}
//Starting the chat
void start_chat(int server, Manager& manage)
{
    std::string message;
    auto recv_msg = manage.recv_message(server);
    if (recv_msg.has_value())
    {
        std::string converted_message = recv_msg.value();
        std::cout << converted_message;
    }

    //User enters nickname
    std::getline(std::cin >> std::ws, message);

    if (message != "/quit")
    {
        manage.send_message(server, message);

        //Welcome message
        std::cout << "Welcome... " << message << "!" << std::endl;

        //Creating the thread to recieve messages
        std::thread recv_loop(recv_chat, server, std::ref(manage));

        //Start sending messages
        while(message != "/quit")
        {
            std::getline(std::cin >> std::ws, message);
            manage.send_message(server, message);
        }

        recv_loop.join();
    }
    else 
    {
        std::cout << "[!] Nickname can't be a command" << std::endl;
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
        std::cout << "[!] Unable to connect to server..." << std::endl;

        return 1;
    }

    //SUCCESS
    std::cout << "[*] Connected to the server...!" << std::endl;

    //Creating thread
    //std::thread start(start_chat, server_fd, std::ref(manage));
    //start.join();
    std::thread recv_loop(recv_chat, server_fd, std::ref(manage));
    recv_loop.join();

    std::cout << "--END CHAT--" << std::endl;
    return 0;
}
