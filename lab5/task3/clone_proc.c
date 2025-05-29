#define _GNU_SOURCE
#include <sched.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

// размер области стека
#define STACK_SIZE (4 * 1024)

// функция 2 - выделяем массив на стеке и рекурсивно вызываем себя
void recursive_func(int depth) {
    char hello_buf[] = "hello world";

    if (--depth > 0) {
        recursive_func(depth);
    }
}

// точка входа для нового процесса
int child_entry(void *arg) {
    // запускаем рекурсивную функцию с глубиной 10
    recursive_func(10);
    // завершаем процесс не вызывая функций завершения - в выводе stackfile.dat видно что стек не перезаписывается функциями, вызываемыми при выходе
    _exit(0);
}

int main(void) {
    const char *file_path = "stackfile.dat";
    int fd = open(file_path, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        printf("open failed");
        exit(EXIT_FAILURE);
    }

    // увеличиваем файл до нужного размера
    if (ftruncate(fd, STACK_SIZE) < 0) {
        printf("ftruncate failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // отображаем файл в память (общий, синхронизируемый)
    void *stack_region = mmap(NULL, STACK_SIZE,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    if (stack_region == MAP_FAILED) {
        printf("mmap failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // стек для дочернего процесса растет вниз, передаем указатель на конец региона
    void *child_stack = (char *)stack_region + STACK_SIZE;

    // создаем процесс clone-ом
    pid_t child_pid = clone(child_entry, child_stack, SIGCHLD, NULL);
    if (child_pid < 0) {
        printf("clone failed");
        munmap(stack_region, STACK_SIZE);
        exit(EXIT_FAILURE);
    }

    // ждем завершения дочернего процесса с идентификатором child_pid
    waitpid(child_pid, NULL, 0);

    // освобождаем область памяти стека
    munmap(stack_region, STACK_SIZE);

    return 0;
}