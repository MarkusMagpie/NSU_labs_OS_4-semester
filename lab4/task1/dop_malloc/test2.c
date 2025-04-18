#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    void *ptr = malloc(2 * 1024 * 1024);
    printf("malloc(2MB) = %p\n", ptr);

    free(ptr);

    return 0;
}