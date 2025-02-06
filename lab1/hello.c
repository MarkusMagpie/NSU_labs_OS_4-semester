#include <stdio.h>
#include <dlfcn.h> // 4

void hello_from_static_lib();
void hello_from_dynamic_lib();

// int main() {
//     // 1
//     // printf("Hello, world!\n");
//     // 2
//     // hello_from_static_lib();
//     // 3
//     hello_from_dynamic_lib();
//     return 0;
// }

// 4
int main() {
    void *lib = dlopen("./libdyn_runtime.so", RTLD_LAZY); // загружаем библиотеку во время выполнения функции
    if (!lib) {
        printf("Error loading library");
        return 0;
    }

    void (*hello)() = dlsym(lib, "hello_from_dyn_runtime_lib"); // получаем указатель на функцию из dyn_runtime_lib.c
    if (!hello) {
        printf("Error 2");
        return 0;
    }

    hello();

    dlclose(lib); // освобождаем библиотеку после использования функции
    return 0;
}