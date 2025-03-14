#include <stdio.h>

int global_initialized = 10; // iv - глобальная инициализированная переменная
int global_uninitialized;    // v - глобальная неинициализированная переменная (тупо объявить?)
const int global_const = 20; // vi - глобальная константа

void function() {
    int local_var = 30;        // i - локальная переменная в функции
    static int static_var = 40; // ii - статическая переменная в функции
    const int local_const = 50;  // iii - константа в функции

    printf("i - адрес локальной переменной: %p\n", (void*)&local_var);
    printf("ii - адрес статической переменной: %p\n", (void*)&static_var);
    printf("iii - адрес константной переменной: %p\n", (void*)&local_const);
}

int main() {
    function();

    printf("iv - адрес глобальной переменной: %p\n", (void*)&global_initialized);
    printf("v - адрес глобальной неинициализированной переменной: %p\n", (void*)&global_uninitialized);
    printf("vi - адрес глобальной константы: %p\n", (void*)&global_const);

    return 0;
}
