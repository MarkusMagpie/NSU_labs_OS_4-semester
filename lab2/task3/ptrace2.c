#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/wait.h> // waitpid + макросы WSTOPSIG, WIFEXITED
#include <sys/user.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_THREADS 1024

// https://filippo.io/linux-syscall-table/
const char *syscall_names[] = {
    "read", "write", "open", "close", "stat", "fstat", "lstat", "poll", "lseek",
    "mmap", "mprotect", "munmap", "brk", "rt_sigaction", "rt_sigprocmask", "rt_sigreturn",
    "ioctl", "pread64", "pwrite64", "readv", "writev", "access", "pipe", "select", [230] = "nanosleep"
};

int main() {
    pid_t child = fork();
    if (child == -1) {
        printf("fork error");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        // дочерний процесс: разрешаем трассировать себя родительским процессом (ptrace()) и запускаем цель (exec())
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        // замена дочернего процесса на выполение программы potoks.c
        execl("./potoks", "potoks", NULL);
        return EXIT_FAILURE;
    }

    // ожидаем остановки дочернего процесса (дожидаюсь выполнения exec() у ребенка)
    int status;
    waitpid(child, &status, 0); // ребенок: exec() - ядро остановит ребенка и отправит родителю сигнал SIGTRAP - сигнал мне для трассировки 

    // PTRACE_SETOPTIONS - установка опций трассировки в параметре data:
    // PTRACE_O_TRACECLONE - трассировать новые потоки созданные через clone() в potoks.c
    if (ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACECLONE) == -1) {
        printf("ptrace PTRACE_SETOPTIONS error");
        exit(1);
    }

    // массив с индикаторами входа/выхода потоков из системных вызовов
    int in_syscall[MAX_THREADS] = {0}; 

    while (1) {
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        // ждем события от любого потока (включая новые созданные через clone):
        // -1 - ждать любого дочернего процесса или потока
        pid_t tid = waitpid(-1, &status, NULL);

        // дочерний поток завершился нормально 
        if (WIFEXITED(status)) {
            printf("thread %d exited with status %d\n", tid, WEXITSTATUS(status));
            if (tid == child) break; // завершился main поток дочернего процесса
            continue;
        }

        // дочерний поток остановился
        if (WIFSTOPPED(status)) {
            int sig = WSTOPSIG(status); // сигнал вызвавший остановку

            if (sig == SIGTRAP) {
                struct user_regs_struct regs;
                if (ptrace(PTRACE_GETREGS, tid, NULL, &regs) == -1) {
                    printf("ptrace PTRACE_GETREGS error");
                    continue;
                }

                long syscall_num = regs.orig_rax; // индекс в syscall_names
                int idx = tid % MAX_THREADS; // номера потоков 

                // если еще не в системном вызове (in_syscall[idx] == 0)
                if (!in_syscall[idx]) {
                    in_syscall[idx] = 1;
                    if (syscall_num >= 0 && syscall_names[syscall_num] != NULL) {
                        printf("thread %d: enter syscall %s (%ld)\n", tid, syscall_names[syscall_num], syscall_num);
                    } else {
                        printf("thread %d: enter unknown syscall %ld\n", tid, syscall_num);
                    }
                } else { // если уже в системном вызове
                    in_syscall[idx] = 0;
                    if (syscall_num >= 0 && syscall_names[syscall_num] != NULL) {
                        printf("thread %d: exit syscall %s (%ld), return = %llu\n", tid, syscall_names[syscall_num], syscall_num, regs.rax);
                    } else {
                        printf("thread %d: exit unknown syscall %ld, return = %llu\n", tid, syscall_num, regs.rax);
                    }
                }
            }
        }

        ptrace(PTRACE_SYSCALL, tid, NULL, NULL);
    }

    return EXIT_SUCCESS;
}