#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t child_pid = fork();
    
    if (child_pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
        printf("[CHILD] PID: %d\n", getpid());
        printf("[CHILD] старый PPID: %d\n", getppid());
        sleep(30);
        printf("[CHILD] новый PPID: %d\n", getppid());
        printf("[CHILD] завершен!\n");
        exit(0);
    } else { 
        printf("[PARENT] PID: %d\n", getpid());
        printf("[PARENT] завершен!\n");
        exit(0); 
    }
    
    return 0;
}