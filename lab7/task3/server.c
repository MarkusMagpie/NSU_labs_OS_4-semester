#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // inet_ntoa
#include <sys/select.h>

#define PORT 5000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
    int server_fd, client_sock_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    fd_set read_fds;
    int max_fd;
    int client_sockets[MAX_CLIENTS];
    int i;

    // инициализирую клиентские сокеты
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // настроил повторное использование порта - SO_REUSEADDR позволяет переиспользовать порт сразу после завершения сервера. даже если он в состоянии TIME_WAIT
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        printf("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // настройка адреса сервера server_addr
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET; // IP адрес к которому будет привязан сокет (IPv4)
    server_addr.sin_port = htons(PORT); // номер порта (в сетевом порядке байт) к которому будет привязан сокет

    // привязка сокета к адресу server_addr
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // начало прослушивания (5 потому что в мане прочитал что обычно <= 5)
    if (listen(server_fd, 5) < 0) {
        printf("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TCP сервер запущен на порту %d (используем select())\n", PORT);

    // НОВОЕ: инициализация наборов дескрипторов
    FD_ZERO(&read_fds); // занулил множество read_fds, т.е. пометил все дескрипторы как "не отслеживаемые"
    FD_SET(server_fd, &read_fds); // добавил слушающий сокет server_fd в множество read_fds. будем следить за тем, когда на server_fd появится новое входящее соединение
    max_fd = server_fd; // наибольший номер дескриптора в множестве. select() ТРЕБУЕТ знать диапазон проверяемых дескрипторов - от 0 до max_fd включительно!!!

    // бесконечно ждем активности на любом из отслеживаемых сокетов
    while (1) {
        fd_set temp_fds = read_fds; // копия множества read_fds, так как select() модифицирует переданное множество
        
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
            printf("select error");
            exit(EXIT_FAILURE);
        }

        // обработка нового подключения (FD_ISSET - есть ли активность на server_fd (попытка полключения)?)
        if (FD_ISSET(server_fd, &temp_fds)) {
            client_sock_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_sock_fd < 0) {
                printf("accept error");
                continue;
            }

            printf("новое подключение (клиент) от %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // добавляю новый сокет client_sock_fd в массив
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_sock_fd;
                    FD_SET(client_sock_fd, &read_fds);
                    if (client_sock_fd > max_fd) {
                        max_fd = client_sock_fd;
                    }
                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                printf("достигнут лимит клиентов (i == MAX_CLIENTS)\n");
                close(client_sock_fd);
            }
        }

        // обработка активности клиентов (проходим по всем активным сокетам из client_sockets)
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            // FD_ISSET(sd, &temp_fds) <=> на этом конкретном клиентском сокете есть данные от клиента!!!
            if (sd > 0 && FD_ISSET(sd, &temp_fds)) {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = recv(sd, buffer, BUFFER_SIZE, 0);

                if (bytes_read <= 0) {
                    // закрываю соединения
                    getpeername(sd, (struct sockaddr*)&client_addr, &client_len);
                    printf("Клиент отключен: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    // закрытие соединения + очистка элемента массива происходит в обоих случаях
                    close(sd);
                    FD_CLR(sd, &read_fds);
                    client_sockets[i] = 0;
                } else {
                    // эхо-ответ
                    buffer[bytes_read] = '\0';
                    printf("получил от клиента %d: %s\n", sd, buffer);
                    if (send(sd, buffer, bytes_read, 0) < 0) {
                        printf("send error");
                        close(sd);
                        FD_CLR(sd, &read_fds);
                        client_sockets[i] = 0;
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}