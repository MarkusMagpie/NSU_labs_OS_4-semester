#include <stdio.h>
#include <unistd.h>
#include <time.h>


#define ITERATIONS 100  // много для наглядности

// 10 * 100 сисколов
void test_multiple_syscalls() {
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < 10; j++) {
            write(1, "X", 1);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    
    printf("\n10 * 100 сисколов (без буфера): %.3f ms\n", elapsed_ns / 1e6);
}

// 100 сисколов
void test_buffered() {
    struct timespec start, end;
    char buf[10] = "XXXXXXXXXX";  // буфер на 10 символов

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        write(1, buf, 10);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    
    printf("\n100 сисколов (с буфером): %.3f ms\n", elapsed_ns / 1e6);
}

void test_fputs() {
    fputs("AAAAA", stdout);
    fputs("BBBBB", stdout);
}

int main() {
    test_multiple_syscalls();
    test_buffered();

    test_fputs();
    return 0;
}