#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char *argv[], char *envp[]) {
    printf("envp pointer:    %p\n", (void*)envp);
    printf("environ pointer: %p\n", (void*)environ);
    printf("=== envp ===\n");
    for (char **e = envp; *e; ++e) printf("%s\n", *e);
    printf("\n=== environ ===\n");
    for (char **e = environ; *e; ++e) printf("%s\n", *e);

    setenv("b", "3", 1); // ожидай изменений только в environ

    printf("\n\nenvp pointer:    %p\n", (void*)envp);
    printf("environ pointer: %p\n", (void*)environ);
    printf("=== envp ===\n");
    for (char **e = envp; *e; ++e) printf("%s\n", *e);
    printf("\n=== environ ===\n");
    for (char **e = environ; *e; ++e) printf("%s\n", *e);

    setenv("c", "4", 1); // ожидай изменений только в environ

    printf("\n\nenvp pointer:    %p\n", (void*)envp);
    printf("environ pointer: %p\n", (void*)environ);
    printf("=== envp ===\n");
    for (char **e = envp; *e; ++e) printf("%s\n", *e);
    printf("\n=== environ ===\n");
    for (char **e = environ; *e; ++e) printf("%s\n", *e);

    return 0;
}