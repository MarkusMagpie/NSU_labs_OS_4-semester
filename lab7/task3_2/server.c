#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

#define OUTBUF_SIZE (1<<16)  // 64 KB per-client

// НОВОЕ - инфа о каждои подключенном к серверу клиенте
typedef struct {
    int fd; // дескриптор клиентского сокета
    char outbuf[OUTBUF_SIZE]; // кольцевой буфер для исходящих данных
    size_t out_head; // индекс в outbuf - первый байт, который ещё не был отправлен клиенту
    size_t out_tail; // индекс в outbuf после последнего скопированного байта  
} client_t;

// перевод сокета в неблокирующий режим с помощью fcntl и флага O_NONBLOCK
int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void) {
    int server_fd, max_fd, i, client_sock_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    client_t clients[MAX_CLIENTS] = {0};
    fd_set master_read, master_write;

    // 1 - создаём слушающий TCP сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("socket error");
        exit(EXIT_FAILURE);
    }

    // настроил повторное использование порта - SO_REUSEADDR позволяет переиспользовать порт сразу после завершения сервера. даже если он в состоянии TIME_WAIT
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        printf("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // НОВОЕ
    make_nonblocking(server_fd);

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

    // начало прослушивания сокета в порту 5000
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        printf("listen error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TCP сервер запущен на порту %d (используем select())\n", PORT);

    // 2 - инициализируем наборы дескрипторов
    FD_ZERO(&master_read); // занулил множество master_read, т.е. пометил все дескрипторы как "не отслеживаемые"
    FD_ZERO(&master_write);
    FD_SET(server_fd, &master_read); // добавил слушающий сокет server_fd в множество master_read. будем следить за тем, когда на server_fd появится новое входящее соединение
    max_fd = server_fd; // наибольший номер дескриптора в множестве. select() ТРЕБУЕТ знать диапазон проверяемых дескрипторов - от 0 до max_fd включительно!!!

    // 3 - основной цикл
    // бесконечно ждем активности на любом из отслеживаемых сокетов
    while (1) {
        fd_set temp_read = master_read; // копия множества master_read, так как select() модифицирует переданное множество (temp_read)
        fd_set temp_write = master_write;

        if (select(max_fd + 1, &temp_read, &temp_write, NULL, NULL) < 0) {
            printf("select error\n");
            break;
        }
        // temp_read, temp_write модифицированы - выставлены биты только для тех дескрипторов, на которых есть готовность к чтению и записи соответственно

        // 4 - обработка новых подключений
        // обработка нового подключения (FD_ISSET - установлен ли бит для именно слушающего сокета server_fd?)
        // да -> в очереди есть входящее подключение (connect() от клиента)
        if (FD_ISSET(server_fd, &temp_read)) {
            while ((client_sock_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) >= 0) {
                if (client_sock_fd < 0) {
                    printf("accept error");
                    continue;
                }

                make_nonblocking(client_sock_fd);

                printf("новое подключение (клиент) от %s:%d (fd=%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_sock_fd);

                // добавляю новый сокет client_sock_fd в массивы (установил по биту в master_read, master_write)
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd <= 0) {
                        clients[i].fd = client_sock_fd;
                        clients[i].out_head = 0;
                        clients[i].out_tail = 0;
                        FD_SET(client_sock_fd, &master_read);
                        FD_SET(client_sock_fd, &master_write);
                        if (client_sock_fd > max_fd) {
                            max_fd = client_sock_fd;
                        }
                        break;
                    }
                }

                if (i == MAX_CLIENTS) {
                    printf("достигнут лимит клиентов (i == MAX_CLIENTS). Отклоняю соединение fd=%d\n", client_sock_fd);
                    close(client_sock_fd);
                }
            }

            // НОВОЕ благодаря флагу O_NONBLOCK - если accept вернул -1: то errno == EAGAIN/EWOULDBLOCK
            // считаю любой другой случай ошибкой и вывожу сообщение об этом
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                printf("accept error");
            }
        }

        // 5 - обработка активности клиентов (проходим по всем активным сокетам из client_sockets)
        for (i = 0; i < MAX_CLIENTS; i++) {
            client_t *c = &clients[i];
            int fd = c->fd;

            if (fd <= 0) continue;

            // 5.1 - обработка дескрипторов готовых к чтению
            // FD_ISSET(fd, &temp_read) <=> на этом конкретном клиентском сокете fd есть данные от клиента
            if (FD_ISSET(fd, &temp_read)) {
                char buf[BUFFER_SIZE];
                ssize_t bytes_read = recv(fd, buf, sizeof(buf), 0);
                if (bytes_read <= 0) {
                    // закрываю соединения + очистка элемента массива master_read, master_write
                    FD_CLR(fd, &master_read);
                    FD_CLR(fd, &master_write);

                    getpeername(fd, (struct sockaddr*)&client_addr, &client_len);
                    printf("клиент отключен: %s:%d (fd=%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), fd);

                    close(fd);
                    c->fd = 0;

                    continue;
                }

                // запись данных в кольцевой буфер outbuf (эхо-ответ)
                size_t used = (c->out_tail + OUTBUF_SIZE - c->out_head) % OUTBUF_SIZE;
                size_t free_space = OUTBUF_SIZE - used;
                size_t to_copy = bytes_read < free_space ? bytes_read : free_space; // сколько байт нужно скопировать в outbuf
                size_t tail = c->out_tail;
                size_t part1 = to_copy < (OUTBUF_SIZE - tail) ? to_copy : (OUTBUF_SIZE - tail); // сколько из to_copy можно скопировать до конца outbuf
                
                // копирование part1 до конца outbuf, потом копирование невлезших to_copy - part1 с начала outbuf
                memcpy(c->outbuf + tail, buf, part1);
                memcpy(c->outbuf, buf + part1, to_copy - part1);
                
                c->out_tail = (tail + to_copy) % OUTBUF_SIZE;
            }

            // 5.2 - обработка дескрипторов готовых к записи
            // FD_ISSET(fd, &temp_write) <=> на этом конкретном клиентском сокете (fd) есть возможность отправить данные
            if (FD_ISSET(fd, &temp_write)) {
                // число байт, которые уже скопированы в буфер и ещё не отправлены клиенту
                size_t used = (c->out_tail + OUTBUF_SIZE - c->out_head) % OUTBUF_SIZE;
                if (used > 0) {
                    size_t head = c->out_head;
                    size_t chunk = used < (OUTBUF_SIZE - head) ? used : (OUTBUF_SIZE - head);
                    // благодаря O_NONBLOCK send() никогда не будет висеть в ожидании освобождения буфера
                    // вместо ожидания send() вернет errno == EAGAIN или errno == EWOULDBLOCK
                    ssize_t bytes_send = send(fd, c->outbuf + head, chunk, 0);
                    printf("отправлено сообщение: %s\n", c->outbuf + head);
                    if (bytes_send > 0) {
                        c->out_head = (head + bytes_send) % OUTBUF_SIZE;
                    } else if (bytes_send <= 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
                        FD_CLR(fd, &master_read);
                        FD_CLR(fd, &master_write);
                        close(fd);
                        printf("ошибка send, не связана с блокировкой: %s\n", strerror(errno));
                        c->fd = 0;
                    }
                }
            }
        }
    }

    // 6 - освобождение ресурсов
    close(server_fd);
    return 0;
}
