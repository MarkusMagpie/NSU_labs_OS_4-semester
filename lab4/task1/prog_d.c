#include <stdio.h>
#include <stdlib.h>

int* global_vatiable_address = NULL;

void create_local_variable() {
    int local_var = 42; // локальная переменная, создается на стеке
    global_vatiable_address = &local_var;

    // int* local_var = malloc(sizeof(int)); // создал указатель local_var на блок памяти в куче (heap) размером sizeof(int)
    // *local_var = 42; // в ячейку памяти в куче на которую ссылается local_var записываем 42
    // return local_var; // возвращение адреса выделенного блока памяти
}

int main() {
    create_local_variable();
    printf("адрес переменной: %p; \nзначение по этому адресу: %d\n", (void*)global_vatiable_address, *global_vatiable_address);

    // free(ptr);
    return 0;
}
