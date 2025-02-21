#include <stdio.h>
#include <unistd.h> // ii + iii
// #include <sys/syscall.h> // iii для SYS_write

int main() {
    // i
    // printf("Hello, world!\n");
    // ii
    const char msg[] = "Hello, world!\n";
    write(1, msg, sizeof(msg) - 1);
    // iii
    // const char msg[] = "Hello, world!\n";
    // syscall(SYS_write, 1, msg, sizeof(msg) - 1);

    return 0;
}