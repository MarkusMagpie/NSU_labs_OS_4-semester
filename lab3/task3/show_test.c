#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

// функция для получения PFN по виртуальному адресу
uint64_t get_pfn(void* addr) {
    long page_size = sysconf(_SC_PAGESIZE);
    long index = ((unsigned long)addr) / page_size;
    char path[64];
    sprintf(path, "/proc/self/pagemap");
    
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        printf("fopen error");
        exit(1);
    }
    
    fseek(f, index * 8, SEEK_SET); // каждая запись в pagemap занимает 8 байт
    uint64_t entry;
    size_t read = fread(&entry, sizeof(uint64_t), 1, f);
    fclose(f);
    
    if (read != 1) {
        printf("не удалось прочитать pagemap\n");
        return 0;
    }
    
    if (entry & (1ULL << 63)) { // присутствует ли страница?
        return entry & ((1ULL << 55) - 1); // извлечение PFN (биты 0-54)
    } else {
        return 0;
    }
}

int main() {
    // выделение памяти
    void* ptr = malloc(1024);
    if (ptr == NULL) {
        printf("malloc");
        return EXIT_FAILURE;
    }
    memset(ptr, 0, 1024); // нициализация памяти нулями

    pid_t pid = fork();
    if (pid < 0) {
        printf("fork error");
        exit(1);
    } else if (pid == 0) {        
        // дочерний процесс прооизводит запись в страницу - выделен новый физический фрейм, в который копируется содержимое измененной страницы
        ((char*)ptr)[0] = 'a';

        printf("child PID: %d, PFN после записи: %lu\n", getpid(), get_pfn(ptr));
        
        free(ptr);
        exit(0);
    } else { 
        printf("PPID: %d, начальный PFN: %lu\n", getpid(), get_pfn(ptr));
        
        // ждем завершения ребенка
        wait(NULL);
        
        // вывод PFN после изменения (должен остаться прежним)
        printf("PPID: %d, PFN после записи в дочернем: %lu\n", getpid(), get_pfn(ptr));
        
        free(ptr);
    }

    return EXIT_SUCCESS;
}