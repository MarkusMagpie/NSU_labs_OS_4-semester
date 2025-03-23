#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());  // i. выводит pid процесса;
    sleep(1);                       // ii. ждет 1 секунду;

    if (argc == 1) {                                    // iii. делаем exec НОтолько при первом запуске
        char *args[] = {argv[0], "aaa", NULL};
        // char *args;
        printf("exec выполняется ...\n");
        execvp(argv[0], args);                          // заменяем процесс на новый экземпляр себя
        perror("exec не удался");
        return 0;
    }

    printf("Hello world\n");       // iv. выводит сообщение “Hello world” - после успешного выполнения exec
    sleep(60);                     // за это время делай cat /proc/<pid>/maps
    return 0;
}