#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close и тд
#include <dirent.h> // для работы с директориями: opendir, readdir, closedir
#include <sys/stat.h> // mkdir
#include <libgen.h> // basename
#include <fcntl.h> // O_WRONLY, O_CREAT, O_TRUNC и тд

// прототипы функций
// a-c - создать/вывести содержимое/удалить каталог
int create_dir(char *argv[]);
int list_dir(char *argv[]);
int remove_dir(char *argv[]);

// d-f - создать/вывести содержимое/удалить файл
int create_file(char *argv[]);
int print_file(char *argv[]);
int remove_file(char *argv[]);

// // g-j - создать symlink/вывести содержимое symlink/вывести содержимое файла на который указывает symlink/удалить symlink
int create_symlink(int argc, char *argv[]);
// int print_symlink(int argc, char *argv[]);
// int print_symlink_file(int argc, char *argv[]);
// int remove_symlink(int argc, char *argv[]);

// // k-n - создать hardlink/удалить hardlink/вывести права доступа к файлу и количество ссылок на него/изменить права доступа к файлу
// int create_hardlink(int argc, char *argv[]);
// int remove_hardlink(int argc, char *argv[]);
// int print_file_info(int argc, char *argv[]);
// int change_file_rights(int argc, char *argv[]);



// реализации функций
// a - создать каталог, переданный аргументом
int create_dir(char *argv[]) {
    if (mkdir(argv[1], 0755) != 0) {
        printf("ошибка mkdir при создании каталога: %s\n", argv[1]);
        return -1;
    }

    return 0;
}

// b - вывести содержимое каталога, указанного в аргументе
int list_dir(char *argv[]) {
    DIR *dir = opendir(argv[1]);
    if (!dir) {
        printf("ошибка opendir при открытии каталога: %s\n", argv[1]);
        return -1;
    }

    printf("Содержимое каталога: %s\n", argv[1]);

    struct dirent *entry;
    /* логика: readdir-ом читаю по элементу исходного каталога dir, в entry лежит укзаатель на структуру dirent
       dirent есть стракт в котором лежит 2 поля: d_ino - file serial number; d_name - name of file
    */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}

// c - удалить каталог, указанный в аргументе
int remove_dir(char *argv[]) {
    if (rmdir(argv[1]) != 0) {
        printf("ошибка rmdir при удалении каталога: %s\n", argv[1]);
        return -1;
    }

    return 0;
}

// d - создать файл, указанный в аргументе
int create_file(char *argv[]) {
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd == -1) {
        printf("ошибка open при создании файла: %s\n", argv[1]);
        return -1;
    }

    close(fd);
    return 0;
}

// e - вывести содержимое файла, указанного в аргументе
int print_file(char *argv[]) {
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("ошибка fopen при открытии файла: %s\n", argv[1]);
        return -1;
    }

    printf("Содержимое файла: %s\n", argv[1]);

    char c;
    while ((c = fgetc(file)) != EOF) {
        printf("%c", c);
    }

    fclose(file);
    return 0;
}

// f - удалить файл, указанный в аргументе
int remove_file(char *argv[]) {
    if (remove(argv[1]) != 0) {
        printf("ошибка remove при удалении файла: %s\n", argv[1]);
        return -1;
    }

    return 0;
}

// g - создать символьную ссылку на файл, указанный в аргументе
int create_symlink(int argc, char *argv[]) {
    if (symlink(argv[1], argv[2])) {
        printf("ошибка symlink при создании символьной ссылки: %s -> %s\n", argv[1], argv[2]);
        return -1;
    }

    return 0;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("неверное количество аргументов\n");
        return -1;
    }

    char *cmd = basename(argv[0]);
    // printf("Команда: %s\n", cmd);
    
    if (strcmp(cmd, "create_dir") == 0) {
        if (create_dir(argv) == 0) {
            printf("\nКоманда create_dir успешно создала каталог: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "list_dir") == 0) {
        if (list_dir(argv) == 0) {
            printf("\nКоманда list_dir успешно вывела содержимое каталога: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "remove_dir") == 0) {
        if (remove_dir(argv) == 0) {
            printf("\nКоманда remove_dir успешно удалила каталог: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "create_file") == 0) {
        if (create_file(argv) == 0) {
            printf("\nКоманда create_file успешно создала файл: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "read_file") == 0) {
        if (print_file(argv) == 0) {
            printf("\nКоманда read_file успешно вывела содержимое файла: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "remove_file") == 0) {
        if (remove_file(argv) == 0) {
            printf("\nКоманда remove_file успешно удалила файл: %s\n", argv[1]);
        }
    } else if (strcmp(cmd, "create_symlink") == 0) {
        if (create_symlink(argc, argv) == 0) {
            printf("\nКоманда create_symlink успешно создала символьную ссылку: %s -> %s\n", argv[1], argv[2]);
        }
    } else {
        printf("Неизвестная команда!\n");
        return -1;
    }

    return 0;
}