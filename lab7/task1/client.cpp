#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 5000

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // создаем UDP сокет
    /*
    socket() создает сокет, возвращает дескриптор сокета
        __domain - AF_INET - протокол IPv4
        __type - SOCK_DGRAM - UDP
        __protocol - 0 - выбирается автоматически
    */
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // настройка адреса сервера server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cout << "inet_pton failed" << std::endl;
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "введите сообщение серверу: ";
    std::cin.getline(buffer, BUFFER_SIZE);

    /*
    sendto() отправляет сообщение серверу через сокет: 
        client_fd - дескриптор сокета
        buffer - указатель на буфер с данными
        strlen(buffer) - размер сообщения
        0 - флаги
        (sockaddr*)&server_addr - адрес получателя 
        sizeof(server_addr) - его размер соответственно
    */
    ssize_t sent_len = sendto(client_fd, buffer, strlen(buffer), 0,
        (sockaddr*)&server_addr, sizeof(server_addr));
    
    if (sent_len < 0) {
        std::cout << "sendto failed" << std::endl;
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    /*
    recfvrom() получает сообщение от сервера через сокет:
        client_fd - дескриптор сокета
        buffer - буфер для данными
        BUFFER_SIZE - сколько байт считать из буффера 
        0 - флаги
        NULL - адрес получателя
        NULL - размер получателя
    */
    ssize_t recv_len = recvfrom(client_fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len < 0) {
        std::cout << "recvfrom failed" << std::endl;
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    buffer[recv_len] = '\0';

    std::cout << "получено эхо-сообщение: " << buffer << std::endl;

    close(client_fd);
    return 0;
}