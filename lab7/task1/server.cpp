#include <iostream>
#include <cstring> // memset
#include <unistd.h>
#include <arpa/inet.h> // inet_ntoa

#define BUFFER_SIZE 1024
#define PORT 5000

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // создаем UDP сокет
    /*
    socket() создает сокет, возвращает дескриптор сокета
        __domain - AF_INET - протокол IPv4
        __type - SOCK_DGRAM - UDP
        __protocol - 0 - выбирается автоматически
    */
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        std::cout << "socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // настройка адреса сервера server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IP адрес к которому будет привязан сокет (IPv4)
    server_addr.sin_port = htons(PORT); // номер порта (в сетевом порядке байт) к которому будет привязан сокет

    // привязка сокета к адресу server_addr
    if (bind(server_fd, (sockaddr*)&server_addr, 
            sizeof(server_addr)) < 0) {
        std::cout << "bind failed" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "сервер запущен на порту: " << PORT << std::endl;

    while (true) {
        // принимаем данные от клиента
        /*
        recfvrom() получает сообщение от клиента через сокет:
            server_fd - дескриптор сокета
            buffer - буфер для данными
            BUFFER_SIZE - сколько байт считать из буффера 
            0 - флаги
            (sockaddr*)&client_addr - адрес получателя
            &client_len - размер  получателя
        */
        ssize_t msg_len = recvfrom(server_fd, buffer, BUFFER_SIZE, 0,
            (sockaddr*)&client_addr, &client_len);
        
        if (msg_len < 0) {
            std::cout << "recvfrom failed" << std::endl;
            continue;
        }

        buffer[msg_len] = '\0';
        std::cout << "получено от " 
                  << inet_ntoa(client_addr.sin_addr) << ":"
                  << client_addr.sin_port << " - " 
                  << buffer << std::endl;

        // отправляем данные обратно клиенту (эхо-ответ который видим с клиентской стороны)
        if (sendto(server_fd, buffer, msg_len, 0,
            (sockaddr*)&client_addr, client_len) < 0) {
            std::cout << "sendto failed" << std::endl;
        }
    }

    close(server_fd);
    return 0;
}