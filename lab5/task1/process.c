#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

// i. создает и инициализирует переменную (можно две: локальную и глобальную);
int global_var = 10;

int main() {
    // i.
    int local_var = 20;
    
    // ii. выводит ее (их) адрес(а) и содержимое;
    printf("адрес глобальной: %p, значение: %d\n", &global_var, global_var);
    printf("адрес локальной: %p, значение: %d\n", &local_var, local_var);
    
    // iii. выводит pid;
    printf("PID: %d\n", getpid());

    // iv. порождает новый процесс (используйте fork(2)).
    // в дочернем копии переменных родительского
    pid_t pid = fork();
    if (pid == -1) {
        printf("ошибка fork");
        return 1;
    }

    if (pid == 0) { // значит процесс дочерний
        // v. в дочернем процессе выводит pid и parent pid.
        printf("дочерний PID: %d, родительский PPID: %d\n", getpid(), getppid());
        
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
        // ix. в родительском процессе делает sleep(30);
        sleep(30);
        
        // xi. в родительском процессе дожидается завершения дочернего, вычитывает код завершения 
        int status;
        waitpid(pid, &status, 0);
        
        // viii. в родительском процессе выводит содержимое переменных;
        printf("глобальная (после завершения дочернего процесса): %d\n", global_var);
        printf("локальная (после завершения дочернего процесса): %d\n", local_var);
        
        // xi. выводит причину завершения и код завершения если он есть. 
        if (WIFEXITED(status)) {
            printf("код завершения: %d\n", WEXITSTATUS(status));
        } else {
            printf("завершен сигналом: %d\n", WTERMSIG(status));
        }
    }
    
    return 0;
}