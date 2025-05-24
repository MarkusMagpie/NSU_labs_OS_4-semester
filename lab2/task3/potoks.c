
#include <pthread.h> // pthread_t, pthread_create
#include <unistd.h> // write, sleep
#include <stdio.h>
#include <string.h>

void* thread_func(void* arg) {
    int id = *(int*)arg;
    while (1) {
        char msg[32];
        snprintf(msg, sizeof(msg), "thread %d running\n", id); // в msg записываем сообщение
        write(1, msg, strlen(msg)); // syscall write: вывод msg в stdout (1 - fd для stdout)
        sleep(1);
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;
    
    pthread_create(&t1, NULL, thread_func, &id1);
    pthread_create(&t2, NULL, thread_func, &id2);

    while (1) {
        char msg[32] = "MAIN thread running\n";
        write(1, "MAIN thread running\n", strlen(msg));
        sleep(1);
    }
    return 0;
}
