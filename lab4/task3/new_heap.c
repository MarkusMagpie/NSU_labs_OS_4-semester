#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE sysconf(_SC_PAGESIZE)

// структура блока в куче (метаданные, вне пользовательской области)
typedef struct Block {
    size_t size; // размер пользовательской области
    int free; // 1 - свободный, 0 - занято
    struct Block *prev;
    struct Block *next;
} Header; // Header - псквдоним Block

// указатель на начало списка блоков
Header *head = NULL;
// указатель на mmap-регион памяти (1 непрерывный блок)
void *mappedRegion = NULL;

// при 1 вызове - инициализация mmap-региона
int init_region() {
    size_t regionSize = PAGE_SIZE * 4;
    // выделяем 4 страницы памяти напрямую 
    mappedRegion = mmap(NULL, regionSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mappedRegion == MAP_FAILED) {
        printf("mmap failed");
        return -1;
    }

    head = (Header *)mappedRegion;
    // printf("regionSize: %d, sizeof(Header): %d\n", regionSize, sizeof(Header));
    head->size = regionSize - sizeof(Header); // размер блока = весь регион (16 KB) - размер заголовка (32 B)
    head->free = 1;
    head->prev = NULL;
    head->next = NULL;

    return 0;
}

// выделение буфера в size байт (пользовательских), выравнивая по 8
void *my_malloc(size_t size) {
    if (size == 0) {
        printf("my_malloc: нельзя выделить нулевой буфер\n");
        return NULL;
    }

    if (!mappedRegion) {
        if (init_region() < 0) {
            printf("my_malloc: не удалось инициализировать аллокатор\n");
            return NULL;
        }
    }

    // выравнивание запрашиваемого размера до 8 байт
    size = ((size + 7) / 8) * 8;

    // поиск свободного блока
    Header *cur = head;
    while (cur != NULL) {
        if (cur->free && cur->size >= size) {
            // > а не >= чтобы не создать нулевой блок при разделении
            if (cur->size > size + sizeof(Header)) {
                // сдвигаюсь от найденного заголовка свободного блока на sizeof(Header) + запрашиваемый размер байт(size)
                // каждый блок перед пользовательскими данными хранит метаданные
                Header *newBlock = (Header *)((char *)cur + (size + sizeof(Header)));
                newBlock->size = cur->size - (size + sizeof(Header));
                newBlock->free = 1;

                // cur <-> newBlock=cur->next <-> oldNext=newBlock->next
                newBlock->prev = cur;
                newBlock->next = cur->next;
                if (cur->next != NULL) cur->next->prev = newBlock;
                cur->next = newBlock;

                cur->size = size;
            } else {
                // если не получилось разделить блок, выделяю текущий блок целиком
                cur->free = 0;
                return (char *)cur + sizeof(Header);
            }
        } else {
            cur = cur->next;
        }
    }

    // не наишли свободное место под size байт 
    return NULL;
}

// освобождение ранее выделенного блока + объединение соседних свободных блоков
void my_free(void *ptr) {
    if (ptr == NULL) {
        printf("my_free: дан указатель NULL\n");
        return;
    }

    // указатель на заголовок блока который нужно освободить
    Header *freeBlock = (Header *)((char *)ptr - sizeof(Header));

    if (freeBlock->free) {
        printf("my_free: двойное освобождение или освобождение несуществующего блока\n");
        return;
    }

    freeBlock->free = 1;

    // слияние блока с предыдущим
    if (freeBlock->prev && freeBlock->prev->free) {
        // размер предыдущего блока увеличиваю на размер текущего + размер заголовка (он у всех один)
        freeBlock->prev->size += freeBlock->size + sizeof(Header);
        freeBlock->prev->next = freeBlock->next;
        if (freeBlock->next)
            freeBlock->next->prev = freeBlock->prev;
        freeBlock = freeBlock->prev;
    }

    // со следующим
    if (freeBlock->next && freeBlock->next->free) {
        freeBlock->size += freeBlock->next->size + sizeof(Header);
        freeBlock->next = freeBlock->next->next;
        if (freeBlock->next)
            freeBlock->next->prev = freeBlock;
    }
}

// НИЖЕ ТЕСТЫ И MAIN

// 1 - простое выделение и освобождение одного блока
void test_simple_alloc_free() {
    void *p = my_malloc(128);
    if (!p) {
        printf("test_simple_alloc_free: аллокация памяти не удалась\n");
        return;
    }

    memset(p, 'a', 128);
    
    my_free(p);
    printf("test_simple_alloc_free: тест прошел\n");
}

// 2 - несколько блоков и слияние при освобождении
 void test_split_merge() {
    void *a = my_malloc(64);
    void *b = my_malloc(64);
    void *c = my_malloc(64);

    my_free(b);
    my_free(c);

    // b и c освободили -> будет блок размера >= 128
    void *d = my_malloc(128);
    if (!d) {
        printf("test_split_merge: объединение не удалось, тест не прошел\n");
    } else {
        printf("test_split_merge: тест прошел\n");
    }

    my_free(a);
    my_free(d);
}

// 3 - выравнивание по 8
void test_alignment() {
    void *p1 = my_malloc(1);
    void *p2 = my_malloc(7);
    void *p3 = my_malloc(9);

    // (uintptr_t)p1 указатель -> целое значение его адреса
    if (((uintptr_t)p1 % 8) || ((uintptr_t)p2 % 8) || ((uintptr_t)p3 % 8)) {
        printf("test_alignment: ошибка выравнивания, тест не прошел\n");
    } else {
        printf("test_alignment: тест прошел\n");
    }

    my_free(p1);
    my_free(p2);
    my_free(p3);
}

// 4 - максимально возможная аллокация
void test_max_alloc() {
    size_t max_size = PAGE_SIZE * 4 - sizeof(Header);
    void *p = my_malloc(max_size);
    if (p) { 
        printf("test_max_alloc: тест прошел\n"); 
        my_free(p); 
    } else {
        printf("test_max_alloc: максимальная аллокация не удалась, тест не прошел\n");
    }
}

// 5 - двойное освобождение (double free)
void test_double_free() {
    void *p = my_malloc(64);
    if (!p) {
        printf("test_double_free: аллокация не удалась\n");
        return;
    }
    
    // 1 корректно
    my_free(p);
    
    // 2 должно вызвать ошибку
    my_free(p);
    
    printf("test_double_free: тест завершен\n");
}

void run_all_tests() {
    test_simple_alloc_free();
    test_split_merge();
    test_alignment();
    test_max_alloc();
    printf("\n");
    test_double_free();
}

int main() {
    run_all_tests();

    /*
    The region is also automatically unmapped when the process is terminated
    https://linux.die.net/man/2/munmap
    */
    if (mappedRegion) {
        munmap(mappedRegion, PAGE_SIZE * 4);
        mappedRegion = NULL;
    }
    return 0;
}
