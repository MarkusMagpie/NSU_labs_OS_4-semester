#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h> // для struct user_regs_struct
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("параметры для вывода не указаны\n");
        return EXIT_FAILURE;
    }

    pid_t child = fork();
    if (child == -1) {
        printf("fork error");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        // дочерний процесс: разрешаем трассировать себя родительским процессом (ptrace()) и запускаем цель (execvp())
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            printf("ptrace TRACEME error");
            exit(EXIT_FAILURE);
        }

        execvp(argv[1], &argv[1]); // заменяем процесс на новый экземпляр себя (выполняется первый аргумент командной строки)

        printf("execvp error");
        exit(EXIT_FAILURE);
    } else {
        int status;
        struct user_regs_struct regs; // структура, в которую PTRACE_GETREGS запишет все общие регистры x86_64
        int in_syscall = 0; // флаг - до (0) или после (1) точки системного вызова - способ различать 2 вида остановок

        // в родительском процессе дожидаемся завершения дочернего, вычитывая код завершения 
        waitpid(child, &status, 0); // статус завершения дочернего процесса записывается в status
        printf("child exited with status %d\n", WEXITSTATUS(status));

        // проверяем остановлен ли дочерний процесс (true - дочерний процесс остановлен, false - не остановлен)
        while (WIFSTOPPED(status)) {
            // запрашиваем остановку при входе/выходе из syscall
            if (ptrace(PTRACE_SYSCALL, child, NULL, NULL) < 0) {
                printf("ptrace SYSCALL error");
                break;
            }
            waitpid(child, &status, 0); // жду завершения дочернего процесса опять 

            if (!WIFSTOPPED(status)) break;

            if (ptrace(PTRACE_GETREGS, child, NULL, &regs) < 0) {
                printf("ptrace GETREGS error");
                break;
            }

            if (in_syscall == 0) {
                // точка входа в сискол
                printf("syscall %llu\n", regs.orig_rax);
                in_syscall = 1;
            } else {
                // точка выхода из сискол (2 заход)
                printf(" = 0x%llx\n", regs.rax);
                in_syscall = 0;
            }
        }
    }

    return EXIT_SUCCESS;
}
