#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_heap() {
    FILE *f = fopen("/proc/self/maps", "r");
    if (f == NULL) {
        printf("ошибка открытия файла\n");
        return;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), f)) {
        // printf("%s", buf);
        if (strstr(buf, "heap")) {
            printf("%s", buf);
            break;
        }
    }
    fclose(f);
}

int main() {
    // printf("heap до malloc(64):\n");
    // print_heap();

    // printf("\n");

    void *ptr = malloc(64);
    printf("malloc(64) = %p\n", ptr);
    // printf("\nheap после malloc(64):\n");
    // print_heap();

    // printf("\n");
    
    free(ptr);
    // printf("free(64)\n");
    // printf("\nheap после free(64):\n");
    // print_heap();

    return 0;
}