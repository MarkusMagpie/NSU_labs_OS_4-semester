#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PAGE_SIZE (4096)

void writer(unsigned int *shared, int num_elements) {
    unsigned int value = 0;
    int index = 0;
    while (1) {
        shared[index] = value;
        index = (index + 1) % num_elements;
        value = (value + 1) % num_elements; // value сбрасывается до нуля при достижении num_elements
    }
}

void reader(unsigned int *shared, int num_elements) {
    unsigned int expected = 0;
    int index = 0;
    while (1) {
        unsigned int read_value = shared[index];
        if (read_value != expected) {
            printf("ошибка: ожидал %u, получил %u по индексу %d\n", expected, read_value, index);
            break;
        } else {
            printf("получил %u по индексу %d\n", read_value, index);
        }
        expected = (read_value + 1) % num_elements; 
        index = (index + 1) % num_elements;
    }
}

int main() {
    // создаем разделяемую память
    unsigned int *shared = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) {
        printf("mmap failed");
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
        // процесс-писатель (ребенок)
        writer(shared, num_elements);
    } else {
        // процесс-читатель массива 
        reader(shared, num_elements);
    }

    munmap(shared, PAGE_SIZE);
    return 0;
}