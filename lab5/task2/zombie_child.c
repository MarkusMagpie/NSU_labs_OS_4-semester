#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int global_var = 10;

int main() {
    int local_var = 20;

    // ii. выводит ее (их) адрес(а) и содержимое;
    printf("адрес глобальной: %p, значение: %d\n", &global_var, global_var);
    printf("адрес локальной: %p, значение: %d\n", &local_var, local_var);
    
    // iii. выводит pid;
    printf("PARENT PID: %d\n", getpid());

    // iv. порождает новый процесс (используйте fork(2)).
    // в дочернем копии переменных родительского
    pid_t pid = fork();
    if (pid == -1) {
        printf("ошибка fork");
        return 1;
    }

    if (pid == 0) {
        printf("CHILD PID: %d\n", getpid());
        
        // vi. в дочернем процессе выводит адреса и содержимое переменных, созданных в пункте i.
        printf("глобальная (до изменения): %p - %d\n", &global_var, global_var);
        printf("локальная (до изменения): %p - %d\n", &local_var, local_var);
        
        // vii. в дочернем процессе изменяет содержимое переменных и выводит их значение;
        global_var = 50;
        local_var = 60;
        printf("глобальная (после изменения): %d\n", global_var);
        printf("локальная (после изменения): %d\n", local_var);
        
        // x. в дочернем процессе завершается с кодом “5” (exit(2)).
        exit(5);
    } else {
        // убрал waitpid, чтобы дочерний процесс остался зомби

        // родитель не вызывает wait и спит, чтобы продемонстрировать зомби
        printf("родительский процесс спит 30 секунд...\n");
        sleep(30);
        printf("родительский процесс завершается\n");
        // родитель завершается без ожидания дочернего процесса
    }
    
    return 0;
}