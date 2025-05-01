#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PAGE_SIZE (4096)

void writer(int write_fd, int num_elements) {
    unsigned int value = 0;
    while (1) {
        // запись значения в pipe
        if (write(write_fd, &value, sizeof(unsigned int)) == -1) {
            printf("write failed");
            exit(3);
        }
        value = (value + 1) % num_elements; // сброс при достижении 1024
    }
}

void reader(int read_fd, int num_elements) {
    unsigned int expected = 0;
    while (1) {
        unsigned int read_value;
        // чтение значения из pipe
        if (read(read_fd, &read_value, sizeof(unsigned int)) == -1) {
            printf("read failed");
            exit(4);
        }
        if (read_value != expected) {
            printf("ошибка: ожидал %u, получил %u\n", expected, read_value);
            break;
        }
        expected = (read_value + 1) % num_elements;
    }
}

int main() {
    int pipe_fd[2];
    // создаем pipe
    if (pipe(pipe_fd) == -1) {
        printf("pipe failed");
        exit(1);
    }

    // количество элементов в регионе (4096 / 4 = 1024)
    int num_elements = PAGE_SIZE / sizeof(unsigned int);

    pid_t pid = fork();
    if (pid == -1) {
        printf("fork failed");
        exit(2);
    }

    if (pid == 0) {
        // дочерний процесс (читатель)
        close(pipe_fd[1]); // закрываем дескриптор для записи
        reader(pipe_fd[0], num_elements);
    } else {
        // родительский процесс (писатель)
        close(pipe_fd[0]); // закрываем дескриптор для чтения
        writer(pipe_fd[1], num_elements);
    }

    return 0;
}