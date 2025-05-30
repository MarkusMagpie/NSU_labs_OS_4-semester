#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) {
        printf("fopen error");
        return EXIT_FAILURE;
    }

    char line[256];
    printf("=== текстовые сегменты (r-xp) - по определению clean pages ===\n");
    while (fgets(line, sizeof(line), f)) {
        // ищем только сегменты с правами "r-xp"
        if (strstr(line, " r-xp ")) {
            printf("%s", line);
        }
    }

    fclose(f);
    return EXIT_SUCCESS;
}
