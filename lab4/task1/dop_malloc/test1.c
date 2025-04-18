#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    void *ptr = malloc(64);
    printf("malloc(64) = %p\n", ptr);
    
    free(ptr);

    return 0;
}