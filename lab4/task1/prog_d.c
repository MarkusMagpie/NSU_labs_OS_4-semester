#include <stdio.h>
#include <stdlib.h>

int* create_local_variable() {
    int local_var = 42; // локальная переменная, создается на стеке
    return &local_var;

    // int* local_var = malloc(sizeof(int)); // создал указатель local_var на блок памяти в куче (heap) размером sizeof(int)
    // *local_var = 42; // в ячейку памяти в куче на которую ссылается local_var записываем 42
    // return local_var; // возвращение адреса выделенного блока памяти
}

int main() {
    int* ptr = create_local_variable();
    printf("адрес переменной: %p; \nзначение по этому адресу: %d\n", (void*)ptr, *ptr);

    // free(ptr);
    return 0;
}
