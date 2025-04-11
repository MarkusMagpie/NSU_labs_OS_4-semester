#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>

#define PAGE_SIZE 4096
#define PAGELOCK_SIZE (10 * PAGE_SIZE)

void segv_handler(int sig) {
    printf("рапортую о поимке SIGSEGV!!!\n");
    exit(1);
}

void recursive_stack(int depth) {
    char stack_arr[PAGE_SIZE]; // 4096 байт на стеке
    printf("глубина стека %d: %p\n", depth, stack_arr);
    
    if(depth < 3) {
        recursive_stack(depth + 1);
    }
}

int main() {
    // i выводит pid процесса
    int pid = getpid();
    printf("PID: %d\n", pid);
    
    // ii
    printf("Во 2 терминале пиши: watch cat /proc/%d/maps\n", pid);
    sleep(25);

    // iii-iv рекурсивное использование стека
    printf("\n=== iii эксперимент с рекурсивным использованием стека ===\n");
    recursive_stack(0);
    sleep(10);

    // v-vi на каждой итерации цикла выделяем память на куче
    printf("\n=== v эксперимент с кучей ===\n");
    void* heap_ptrs[5];
    for(int i = 0; i < 5; i++) {
        heap_ptrs[i] = malloc(PAGE_SIZE);
        printf("Часть кучи %d: %p\n", i, heap_ptrs[i]);
        sleep(1);
    }
    
    // vii освободите занятую память.
    for(int i = 0; i < 5; i++) {
        free(heap_ptrs[i]);
    }
    printf("\nvii - освободил занятую память\n");
    sleep(2);

    // viii-ix присоедините к процессу еще один регион адресов размером в 10 страниц
    printf("\n=== viii MMAP эксперимент ===\n");
    void* mapped = mmap(NULL, PAGELOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    printf("mapped блок: %p-%p\n", 
           mapped, 
           mapped + PAGELOCK_SIZE);
    sleep(10);

    // x-xi изменение прав доступа и обработка ошибок
    printf("\n=== x изменения прав доступа к mapped блоку ===\n");
    // программа не аварийно завершалается, а обрабатывает ошибку сегментации SIGSEGV
    signal(SIGSEGV, segv_handler);
    
    printf("1. запрет чтения (---p)\n");
    mprotect(mapped, PAGELOCK_SIZE, PROT_NONE);
    // char test = *((char*)mapped);
    
    printf("2. запрет записи (r--p)\n"); 
    mprotect(mapped, PAGELOCK_SIZE, PROT_READ);
    // ((char*)mapped)[1] = 2; // mapped - pointer на void -> заменяем пойтер на пойнтер с типом к которому примерним оператор []
    
    // восстановил права 
    mprotect(mapped, PAGELOCK_SIZE, PROT_READ | PROT_WRITE);
    
    // xii-xiii отсоедините страницы с 4 по 6 в созданном вами регионе.
    printf("\n=== xii отсоединение страниц ===\n");
    void* partial_addr = (char*)mapped + 3*PAGE_SIZE; // указатель на начало 4-й страницы
    munmap(partial_addr, 3*PAGE_SIZE);
    printf("4-6: %p-%p\n",
           partial_addr,
           partial_addr + 3*PAGE_SIZE);
    sleep(10);

    munmap(mapped, 3*PAGE_SIZE);
    munmap(mapped + 6*PAGE_SIZE, 4*PAGE_SIZE);
    
    return 0;
}