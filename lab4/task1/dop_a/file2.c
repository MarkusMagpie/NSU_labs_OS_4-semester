#include <stdio.h>

extern int global_var; // из file1.c
extern int static_var;

void try() {
    printf("\nИз другого файла:\n");
    printf("global_var: %p\n", (void*)&global_var);
    // printf("static_var: %p\n", (void*)&static_var); // ошибка 
}