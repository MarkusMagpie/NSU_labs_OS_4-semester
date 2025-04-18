#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    printf("Во 2 терминале пиши: watch cat /proc/%d/maps\n", getpid());
    sleep(25);

    int page = 10 * 4096;
    char *p = mmap(NULL, page,
                   PROT_WRITE,              // ТОЛЬКО запись
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);

    sleep(20);

    p[0] = 42; // запись — OK
    printf("значение: %d\n", p[0]);  // считанное значение — НЕ упадёт на моей x86_64

    munmap(p, page);
    return 0;
}
