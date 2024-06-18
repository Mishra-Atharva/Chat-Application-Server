# Chat-Application-Server
This server excels in managing the entire lifecycle of user interactions: from handling incoming connections and gracefully managing disconnections to facilitating seamless message reception and transmission among users. The use of mutexes ensures thread safety, preventing data races and guaranteeing smooth operation under concurrent access scenarios. This comprehensive approach not only enhances the server's performance but also ensures reliability and stability throughout extended usage periods.

## How to use it
- Download the server.cpp file.
- Make changes to the port you want the server to use or the ip you want the server to use.
- Build the file by running this command in the terminal `g++ -std=c++20 -pthread server.cpp -o server`
- Then run the server by running this command in the terminal `./server`
- Once the server is running open another terminal and run the command `nc (ip_address) (port_number)`

## Features to implement in the future
- Client.cpp files
- Encryption
