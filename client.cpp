// C++ program to illustrate the client_socket application in the 
// socket programming 
#include <arpa/inet.h>
#include <cerrno>
#include <cstring> 
#include <functional>
#include <iostream> 
#include <netinet/in.h> 
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h> 
#include <sys/types.h>
#include <thread>
#include <unistd.h> 

class Manager
{
public:
    int send_message(int client_socket, std::string& message)
    {
        ssize_t send_len = send(client_socket, message.c_str(), message.length() + 1, 0);
        return send_len;
    }

    auto recv_message(int client_socket) -> std::optional<std::string>
    {
        char* message = new char[1024];
        ssize_t recv_len = recv(client_socket, message, 1024-1, 0);
        if (recv_len < 1)
        {
            delete[] message;
            return {};
        }
        return std::string(message);
    }
};

auto recv_message(int client_socket) -> std::optional<std::string>
{
    char* message = new char[1024];
    while (true)
    {
        ssize_t recv_len = recv(client_socket, message, 1024-1, 0);
        if (recv_len < 1)
        {
            delete[] message;
        }
        std::cout << message << std::endl;
    }
}

void messaging(int client_socket, Manager& manage)
{
    std::string new_message;
    std::string message;
    auto incoming = manage.recv_message(client_socket);
    if (incoming.has_value())
    {
        std::cout << incoming.value();
        std::getline(std::cin >> std::ws, new_message);
    }
    manage.send_message(client_socket, new_message);
    std::thread receving(recv_message, client_socket);
    while (true)
    {
        std::getline(std::cin >> std::ws, message);
        manage.send_message(client_socket, message);
    }
    receving.join();
}

int main()
{
    Manager manage;
    sockaddr_in socket_address;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(3306);
    socket_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cout << "[!] " << std::strerror(errno) << std::endl;
        return 1;
    }

    int connection = connect(client_socket, (sockaddr*)&socket_address, sizeof(socket_address));
    if (connection == -1)
    {
        std::cout << "[!] " << std::strerror(errno) << std::endl;
        return 1;
    }
    
    std::cout << "[*] Connected!" << std::endl; 

    std::thread start_message(messaging, client_socket, std::ref(manage));
    start_message.join();

    return 0;
}
