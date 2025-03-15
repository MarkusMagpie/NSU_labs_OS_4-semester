#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void buffer_operations() {
    // i. буфер 100 байт
    char* buffer = malloc(100);  
    if (buffer == NULL) {
        printf("ошибка malloc выделения памяти!\n");
        return;
    }

    // ii. записать что-то в буфер
    strcpy(buffer, "hello world");

    // iii. вывести содержимое 
    printf("Буфер 1: %s\n", buffer);

    // iv. освобождаем выделенную память
    free(buffer);
    // buffer = NULL;

    // v. попытка вывести содержимое освобожденного буфера 
    printf("Буфер 1 после освобождения: %s\n", buffer);  // неопределённое поведение

    // vi. выделяем еще один буфер
    buffer = malloc(100);
    if (buffer == NULL) {
        printf("Ошибка выделения памяти!\n");
        return;
    }

    // vii. записываем в новый буфер фразу "hello world"
    strcpy(buffer, "hello world");

    // viii. содержимое нового буфера
    printf("Буфер 2: %s\n", buffer);

    // ix. перемещаем указатель на середину буфера (с 0 на 50 байт)
    char* mid = buffer + 50;

    // x. освобождаем память по новому указателю (ловлю ошибку invalid pointer)
    free(mid);
    // free(buffer);
    // buffer = NULL;

    // xi. вывести содержимое освобожденного буфера
    printf("Буфер после освобождения: %s\n", buffer);  // неопределённое поведение
}

int main() {
    buffer_operations();
    return 0;
}
