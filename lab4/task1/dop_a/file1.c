#include <stdio.h>

int global_var = 10; // глобальная переменная D - external linkage
static int static_var = 20; // статическая глобальная d - internal linkage

void func() {
    static int local_static = 30; // статическая внутри функции d 
    printf("local static: %p\n", (void*)&local_static);
}

int main() {
    printf("global_var: %p\n", (void*)&global_var);
    printf("static_var: %p\n", (void*)&static_var);
    func();
    
    // попытка доступа из другого файла
    extern void try();
    try();
    
    return 0;
}