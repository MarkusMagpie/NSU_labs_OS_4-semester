#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    // дед A создает родителя B
    pid_t pid_b = fork();

    if (pid_b == 0) { 
        // B - родитель для C
        pid_t pid_c = fork();

        if (pid_c == 0) { 
            // дочерний процесс C
            sleep(20);
            printf("C (PID=%d): работаю, мой родитель теперь init (PID=1): %d\n", getpid(), getppid());
            exit(0);
        } else { 
            // B завершается, не вызывая wait() для C
            printf("во 2 терминале пиши: cat /proc/%d/status | grep State\n", getpid());
            sleep(10);
            printf("B (PID=%d): Завершаюсь, становлюсь зомби для A\n", getpid());
            exit(0);
        }
    } else { 
        // A завершается, не вызывая wait() для B
        sleep(15);
        printf("A (PID=%d): завершаюсь, B процесс завершен\n", getpid());
        exit(0);
    }
}