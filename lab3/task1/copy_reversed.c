#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // для работы с директориями: opendir, readdir, closedir
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h> // O_RDONLY
#include <unistd.h> // close и тд

#define BLOCK_SIZE 1024 // это блок для чтения и записи


// переворачивания строки
// возвращаем выделенную память с перевернутой строкой
// ! освободить память после использования!
char* reverse_string(const char *str) {
    int len = strlen(str);
    char *mem = malloc(len + 1);
    if (!mem) {
        printf("ошибка выделения памяти для перевернутой строки маллоком\n");
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        mem[i] = str[len - 1 - i];
    }
    mem[len] = '\0';

    return mem;
}

// новая функция для переворота блока байтов
void reverse_block(char *buffer, size_t size) {
    for (size_t i = 0; i < size / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[size - 1 - i];
        buffer[size - 1 - i] = temp;
    }
}

int copy_file_reversed2(const char *src_path, const char *dest_path) {
    struct stat src_stat;
    if (stat(src_path, &src_stat) < 0) {
        printf("ошибка при получении меттаданных исходного файла: %s\n", src_path);
        return -1;
    }

    // открываем исходный файл для чтения 
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        printf("ошибка open.\n");
        close(src_fd);
        return -1;
    }

    // создал ЦЕЛЕВОЙ файл с теми же правами доступа
    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dest_fd == -1) {
        printf("ошибка при создании целевого файла: %s\n", dest_path);
        close(src_fd);
        return -1;
    }

    // man: Функции truncate и ftruncate устанавливают длину файлового дексриптора dest_fd в src_stat.st_size байт
    int ret = ftruncate(dest_fd, src_stat.st_size); 
    if (ret == -1) {
        printf("ошибка при ftruncate установке размера целевого файла: %s\n", dest_path);
        goto cleanup;
    }

    // буфер для блока
    char *buffer = malloc(BLOCK_SIZE);
    if (buffer == NULL) {
        printf("ошибка выделения памяти для содержимого файла: %s\n", src_path);
        goto cleanup;
    }

    // обрабатываем файл блоками с конца
    size_t remaining = src_stat.st_size; // количество оставшихся байт - сначала это просто размер исходного файла в байтах
    size_t dest_pos = 0;

    while (remaining > 0) {
        // размер текущего блока
        size_t block_size = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        size_t read_pos = remaining - block_size;

        int try_count = 0;
        size_t total_read = 0;
        size_t bytes_read = 0;
        while (bytes_read < block_size && try_count < 10) {
            // чтение блока ИЗ src_fd максимум block_size байтов в буфер buffer с позиции read_pos
            bytes_read = pread(src_fd, buffer, block_size, read_pos);
            if (bytes_read == -1) {
                printf("ошибка при чтении файла: %s\n", src_path);
                goto cleanup;
            }   
            if (bytes_read == 0) break;
            total_read += bytes_read;
            try_count++;
        }
        if (total_read != block_size) {
            printf("ошибка чтения файла (изменился за время чтения?): %s\n", src_path);
            goto cleanup;
        }

        reverse_block(buffer, bytes_read);

        try_count = 0;
        size_t total_written = 0;
        size_t bytes_written = 0;
        while (bytes_written < block_size && try_count < 10) {
            // писание блока в dest_fd максимум bytes_read байтов из буфера buffer начиная со смещения dest_pos 
            bytes_written = pwrite(dest_fd, buffer, bytes_read, dest_pos);
            if (bytes_written == -1) {
                printf("ошибка при записи файла: %s\n", dest_path);
                goto cleanup;
            }
            total_written += bytes_written;
            try_count++;
        }
        if (total_written != block_size) {
            printf("ошибка при записи файла (изменился за время записи?): %s\n", dest_path);
            goto cleanup;
        }

        dest_pos += bytes_written;
        remaining -= bytes_read;
    }

    ret = 0;

cleanup:
    if (buffer) free(buffer);
    if (src_fd != -1) close(src_fd);
    if (dest_fd != -1) close(dest_fd);
    return ret;
}



int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("нужно указать только один аргумент - путь к каталогу\n");
        return 1;
    }

    // существует ли указанный путь и является ли директорией???
    struct stat st;
    if (stat(argv[1], &st) < 0) {
        printf("указанный путь не существует\n");
        return 1;
    }

    if (!S_ISDIR(st.st_mode)) {
        printf("путь не является каталогом: %s\n", argv[1]);
        return 1;
    }

    // ПОЛУЧАЕМ SRC КАТАЛОГ(basename) И РОДИТЕЛЬСКИЙ(dirname) 

    // выделяем имя исходного каталога
    char *src_path_copy = strdup(argv[1]);
    if (!src_path_copy) {
        printf("ошибка выделения памяти для имени каталога: strdup\n");
        return 1;
    }
    char *src_dir_name = basename(src_path_copy); // извлек ПОСЛДЕНИЙ компонент в имени файла (име это аргумент)
    char *reversed_dir_name = reverse_string(src_dir_name);
    if (!reversed_dir_name) {
        free(src_path_copy); // в strdup маллоком выделяли память
        return 1;
    }

    // получаем родительский каталог исходного 
    char *src_path_copy2 = strdup(argv[1]);
    if (src_path_copy2 == NULL) {
        printf("ошибка выделения памяти для имени каталога 2: strdup\n");
        free(src_path_copy);
        free(reversed_dir_name);
        return 1;
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
        return 1;
    }
    // записываю в target_path строку в формате: parent_dir/reversed_dir_name
    snprintf(target_path, target_path_len, "%s/%s", parent_dir, reversed_dir_name); // куда, сколько, что

    // создание целевого каталога с правами доступа исходного
    if (mkdir(target_path, st.st_mode) < 0) {
        printf("ошибка создания (mkdir) целевого каталога %s\n", target_path);
        free(src_path_copy);
        free(src_path_copy2);
        free(reversed_dir_name);
        free(target_path);
        return 1;
    }

    // открытие исходного каталога (не созданного!) для чтения
    DIR *dir = opendir(argv[1]);
    if (!dir) {
        printf("ошибка открытия исходного каталога %s\n", argv[1]); // не существует, нет прав или не каталог 
        free(src_path_copy);
        free(src_path_copy2);
        free(reversed_dir_name);
        free(target_path);
        return 1;
    }

    struct dirent *entry;
    // логика: readdir-ом читаю по элементу исходного каталога dir, в entry лежит укзаатель на структуру dirent
    while ((entry = readdir(dir)) != NULL) {
        // не обращаю внимания - это служебные записи (. - ссылка на текущий каталог, .. - на родительский)
        // ls -a
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

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
            printf("не регулярный исходный файл: %s\n", full_src_path);
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
        if (copy_file_reversed2(full_src_path, full_dest_path) != 0) {
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
