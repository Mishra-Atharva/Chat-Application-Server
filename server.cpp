#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring> 
#include <sstream>
#include <iostream> 
#include <netinet/in.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <sys/socket.h> 
#include <unistd.h> 
#include <thread>
#include <mutex>
#include <set>
#include <optional>

class Manager
{
private:
  std::vector<int> connections;
  std::mutex store_mtx;

public:
  
  //To store the new connections that are joining.
  void store(int client)
  {
    std::unique_lock<std::mutex> store_lock(store_mtx);
    connections.push_back(client);
    for (auto& i : connections)
    {
         std::cout << i << std::endl;
    }
    store_lock.unlock();
  }

  ssize_t send_message(int client, const std::string& message, const std::string& nickname, bool& is_nickname)
  {
    std::unique_lock<std::mutex> send_lock(store_mtx);
    ssize_t result = 0;
    if (is_nickname)
    {
      for (int &user : connections)
      {
        if (user != client)
        {
          ssize_t send_len = send(user, message.c_str(), message.length()+1, 0);
          result = send_len;
        }
      }
    }
    else
    {
      for (int &user : connections)
      {
        if (user != client)
        {
          std::stringstream msg;
          msg << "[" << nickname << "] --> " << message << std::endl;
          std::string template_message = msg.str();
          ssize_t send_len = send(user, template_message.c_str(), template_message.length() + 1, 0);
          result = send_len;
        }
      }
    }
    send_lock.unlock();
    return result;
  }

  auto recv_message(int client) -> std::optional<std::string>
  {
    char* message = new char[2048];
    ssize_t recv_len = recv(client, message, 2048 - 1, 0);
    if (recv_len < 1)
    {
      delete[] message;
      return {};
    }
    return std::string(message);
  }

  void dissconnect(int client)
  {
    std::unique_lock<std::mutex> store_lock(store_mtx);
    std::erase_if(connections, [client](int connection_fd) {
        return connection_fd == client;
    });
    store_lock.unlock();
  }
};

//ConnectionHandler.
void connection_handler(int client_fd, Manager& manager)
{
  //Message controller
  bool is_nickname = true;

  //Message holders.
  std::string join_msg;
  std::string leave_msg;

  //Sending user message to input an nickname.
  char* nickname = new char[1024];
  std::string name_prompt = "Enter Nickname: ";
  ssize_t send_len = send(client_fd, name_prompt.c_str(), name_prompt.length()+1, 0);
  if (send_len < 1)
  {
    delete[] nickname;
    close(client_fd);
    return;
  }

  //Getting the nickname user enters.
  ssize_t recv_len = recv(client_fd, nickname, 1024 - 2, 0);
  if (recv_len < 1)
  {
    delete[] nickname;
    close(client_fd);
    return;
  }

  //removing the \n from the nickname.
  for (size_t i = 0; i < sizeof(nickname); i++)
  {
    switch(nickname[i])
    {
      case '\r':
      case '\n':
        nickname[i] = '\0';
    }
  }

  //Sending all connected users an message that a new user has connected.
  std::stringstream join_alert;
  std::stringstream leave_alert;

  join_alert << "[" << nickname << "] has joined the chat." << std::endl;
  join_msg = join_alert.str();

  
  leave_alert << "[" << nickname << "] has left the chat." << std::endl;
  leave_msg = leave_alert.str();

  manager.store(client_fd);
  manager.send_message(client_fd, join_msg, nickname, is_nickname);
  std::cout << "[~] " << join_msg;
  is_nickname = false;

  while (true)
  {
    auto message = manager.recv_message(client_fd);
    if (message.has_value())
    {
      std::string new_message = message.value();
      manager.send_message(client_fd, new_message, nickname, is_nickname);
    }
    else 
    {
      is_nickname = true;
      manager.send_message(client_fd, leave_msg, nickname, is_nickname);
      manager.dissconnect(client_fd);
      close(client_fd);
      delete[] nickname;
      break;
    }
  }
  std::cout << "[~] " << leave_msg;
}

//Setting up the socket.
int setupSocket()
{
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd == -1)
  {
    std::cout << "[!] "<< std::strerror(errno) << std::endl;
    return -1;
  }

  sockaddr_in socket_address;
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(3306);
  socket_address.sin_addr.s_addr = inet_addr("127.0.0.1");

  int binding = bind(socket_fd, (sockaddr*)&socket_address, sizeof(socket_address));
  if (binding == -1)
  {
    std::cout << "[!] " << std::strerror(errno) << std::endl;
    return -1;
  }

  return socket_fd;
}

int main()
{
  Manager manager;
  int server_fd = setupSocket();

  //Listening to the port
  int listen_to = listen(server_fd, SOMAXCONN);
  if (listen_to == -1)
  {
    std::cout << "[!] " << strerror(errno) << std::endl;
  }

  std::cout << "[~] Listening on port 3306..." << std::endl;

  while (true)
  {
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd == -1)
    {
      std::cout << "[!] " << strerror(errno) << std::endl;
      continue;
    }

    std::thread start_connection(connection_handler, client_fd, std::ref(manager));
    start_connection.detach();
  }
  return 0;
}
