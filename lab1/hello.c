#include <stdio.h>
#include <dlfcn.h> // 4

void hello_from_static_lib();
void hello_from_dynamic_lib();

// int main() {
//     printf("Hello, world!\n");
//     // 2
//     // hello_from_static_lib();
//     // 3
//     // hello_from_dynamic_lib();
//     return 0;
// }

// 4
int main() {
    void *handle = dlopen("./libdyn_runtime.so", RTLD_LAZY); // загружает библиотеку во время выполнения
    if (!handle) {
        printf("Error loading library");
        return 0;
    }

    void (*hello)() = dlsym(handle, "hello_from_dyn_runtime_lib"); // получает указатель на функцию из dyn_runtime_lib.c
    if (!hello) {
        printf("Error finding symbol");
        return 0;
    }

    hello();

    dlclose(handle); // освобождает библиотеку после использования
    return 0;
}