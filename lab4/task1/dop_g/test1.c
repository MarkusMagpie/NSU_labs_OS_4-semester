#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *value = getenv("LANG");

    if (strstr(value, "ru_RU") != NULL) {
        printf("Привет, мир!\n");
    } else if (strstr(value, "en_US") != NULL) {
        printf("Hello, world!\n");
    } else {
        printf("непонятный язык: %s\n", value);
    }

    return 0;
}