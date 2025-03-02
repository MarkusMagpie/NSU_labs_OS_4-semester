#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // для работы с директориями: opendir, readdir, closedir
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>


// переворачивания строки
// возвращаем выделенную память с перевернутой строкой
// ! освободить память после использования!
char* reverse_string(const char *str) {
    int len = strlen(str);
    char *mem = malloc(len + 1);
    if (!mem) {
        printf("ошибка выделения памяти\n");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        mem[i] = str[len - 1 - i];
    }
    mem[len] = '\0';

    return mem;
}

// функция копирования файла с переворотом содержимого
// 0 при успехе и -1 иначе
int copy_file_reversed(const char *src_path, const char *dest_path) {
    FILE *src = fopen(src_path, "rb"); // rb - чтение в бинарном режиме
    if (!src) {
        printf("Ошибка открытия файла: %s\n", src_path);
        return -1;
    }

    // размер файла src_path
    if (fseek(src, 0, SEEK_END) != 0) {
        printf("Ошибка fseek: %s\n", src_path);
        fclose(src);
        return -1;
    }
    long filesize = ftell(src);
    if (filesize < 0) {
        printf("Ошибка ftell (получил отрицательный размер файла): %s\n", src_path);
        fclose(src);
        return -1;
    }
    rewind(src);

    char *buffer = malloc(filesize); // память под все содержимое файла
    if (!buffer) {
        printf("ошибка выделения памяти для содержимого файла: %s\n", src_path);
        fclose(src);
        return -1;
    }
    size_t read_bytes = fread(buffer, 1, filesize, src);
    if (read_bytes != filesize) {
        printf("Ошибка чтения файла: %s\n", src_path);
        free(buffer);
        fclose(src);
        return -1;
    }
    fclose(src);

    // переворачиваем содержимого (средний на месте)
    for (long i = 0; i < filesize/2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[filesize - 1 - i];
        buffer[filesize - 1 - i] = temp;
    }

    FILE *dest = fopen(dest_path, "wb");
    if (!dest) {
        printf("Ошибка создания файла: %s\n", dest_path);
        free(buffer);
        return -1;
    }
    size_t written = fwrite(buffer, 1, filesize, dest);
    if (written != filesize) {
        printf("Ошибка записи файла: %s\n", dest_path);
        free(buffer);
        fclose(dest);
        return -1;
    }
    fclose(dest);
    free(buffer);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("нужно указать только один аргумент - путь к каталогу\n");
        return 0;
    }

    // существует ли указанный путь и является ли директорией???
    struct stat st;
    if (stat(argv[1], &st) < 0) {
        printf("указанный путь не существует\n");
        return 0;
    }
    if (!S_ISDIR(st.st_mode)) {
        printf("путь не является каталогом: %s\n", argv[1]);
        return 0;
    }

    // ПОЛУЧАЕМ SRC КАТАЛОГ(basename) И РОДИТЕЛЬСКИЙ(dirname) 

    // выделяем имя исходного каталога
    char *src_path_copy = strdup(argv[1]);
    if (!src_path_copy) {
        printf("ошибка выделения памяти для имени каталога: strdup\n");
        return 0;
    }
    char *src_dir_name = basename(src_path_copy); // извлек ПОСЛДЕНИЙ компонент в имени файла (име это аргумент)
    char *reversed_dir_name = reverse_string(src_dir_name);
    if (!reversed_dir_name) {
        free(src_path_copy); // в strdup маллоком выделяли память
        return 0;
    }

    // получаем родительский каталог исходного 
    char *src_path_copy2 = strdup(argv[1]);
    if (src_path_copy2 == NULL) {
        printf("ошибка выделения памяти для имени каталога 2: strdup\n");
        free(src_path_copy);
        free(reversed_dir_name);
        return 0;
    }
    char *parent_dir = dirname(src_path_copy2);

    // СОЗДАНИЕ ЦЕЛЕВОГО КАТАЛОГА (С REVERSED ИСХОДНЫМ) 
    
    // полный путь целевого каталога: parent_dir/reversed_dir_name
    size_t target_path_len = strlen(parent_dir) + 1 + strlen(reversed_dir_name) + 1; // +1 для слэша, +1 для \0 в конце
    char *target_path = malloc(target_path_len);
    if (!target_path) {
        printf("ошибка выделения памяти для полного пути целевого каталога\n");
        free(src_path_copy);
        free(src_path_copy2);
        free(reversed_dir_name);
        return 0;
    }
    // записываю в target_path строку в формате: parent_dir/reversed_dir_name
    snprintf(target_path, target_path_len, "%s/%s", parent_dir, reversed_dir_name); // куда, сколько, что

    // создание целевого каталога
    // 0755 - права доступа к директории
    if (mkdir(target_path, 0755) < 0) {
        printf("ошибка создания (mkdir) целевого каталога %s\n", target_path);
        free(src_path_copy);
        free(src_path_copy2);
        free(reversed_dir_name);
        free(target_path);
        return 0;
    }

    // открытие исходного каталога (не созданного!) для чтения
    DIR *dir = opendir(argv[1]);
    if (!dir) {
        printf("ошибка открытия исходного каталога %s\n", argv[1]); // не существует, нет прав или не каталог 
        free(src_path_copy);
        free(src_path_copy2);
        free(reversed_dir_name);
        free(target_path);
        return 0;
    }

    struct dirent *entry;
    // логика: readdir-ом читаю по элементу исходного каталога dir, в entry лежит укзаатель на структуру dirent
    while ((entry = readdir(dir)) != NULL) {
        // не обращаю внимания - это спец записи (. - ссылка на текущий каталог, .. - на родительский)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // полный путь к исходному каталогу (не с развернутым именем как выше делал)
        size_t full_src_path_len = strlen(argv[1]) + 1 + strlen(entry->d_name) + 1;
        char *full_src_path = malloc(full_src_path_len);
        if (!full_src_path) {
            printf("ошибка выделения памяти для полного пути к исходному файлу\n");
            continue;
        }
        snprintf(full_src_path, full_src_path_len, "%s/%s", argv[1], entry->d_name);

        /* ? как проверить что найденный объект - регулярный файл:
           1) функцией stat в struct file_stat получаем информацию о файле
           2) проверяем макросом S_ISREG, что поле st_mode, содержащее режим доступа файла, является regular file (не каталогом)
        */ 
        struct stat file_stat;
        if (stat(full_src_path, &file_stat) < 0) {
            printf("ошибка получения информации о файле: %s\n", full_src_path);
            free(full_src_path);
            continue;
        }
        if (!S_ISREG(file_stat.st_mode)) {
            free(full_src_path);
            continue;
        }

        // переворачиваем имя целевого файла: file1.txt -> txt.1elif
        char *reversed_file_name = reverse_string(entry->d_name);
        if (!reversed_file_name) {
            free(full_src_path);
            continue;
        }

        // создаем полный путь к целевому файлу: target_path + reversed_file_name = full_dest_path 
        // ! target_path аналогично обрабатывал раньше - ПОКАЖИ!
        size_t full_dest_path_len = strlen(target_path) + 1 + strlen(reversed_file_name) + 1;
        char *full_dest_path = malloc(full_dest_path_len);
        if (!full_dest_path) {
            printf("ошибка выделения памяти для полного пути к целевому файлу (с переворотом)\n");
            free(full_src_path);
            free(reversed_file_name);
            continue;
        }
        snprintf(full_dest_path, full_dest_path_len, "%s/%s", target_path, reversed_file_name);

        // full_src_path - путь к оригинальному файлу, 
        // full_dest_path - к перевернутому 
        // осталось скопировать содержимое full_src_path в full_dest_path в обратном порядке
        if (copy_file_reversed(full_src_path, full_dest_path) != 0) {
            fprintf(stderr, "Не удалось скопировать файл: %s\n", full_src_path);
        }

        free(full_src_path);
        free(reversed_file_name);
        free(full_dest_path);
    }

    closedir(dir);
    free(src_path_copy);
    free(src_path_copy2);
    free(reversed_dir_name);
    free(target_path);
    
    return 0;
}
