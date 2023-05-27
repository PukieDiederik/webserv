#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "HttpRequest.hpp"

int main(int argc, char **argv)
{
    (void) argc, (void) argv;
    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // Bind the socket to a port
    int port = 3457;
    struct sockaddr_in server_address;
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << "\n";
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket\n";
        return 1;
    }

    // Accept incoming connections
    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
            std::cerr << "Failed to accept incoming connection\n";
            continue;
        }

        // Read data from the client
        char buffer[1024];
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            std::cerr << "Failed to receive data from client\n";
            close(client_socket);
            continue;
        }

        HttpRequest hr(buffer);
        std::cout << hr.toString();

        // Send a response back to the client
        const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!\r\n";
        ssize_t bytes_sent = send(client_socket, response, std::strlen(response), 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send response to client\n";
        }

        // Close the client socket
        close(client_socket);
    }

    // Close the server socket
    close(server_socket);
}
