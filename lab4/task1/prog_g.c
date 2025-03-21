#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *env_var = "ENV_VAR";

    // g. Заведите переменную окружения со значением "aaa"
    setenv(env_var, "aaa", 0);

    printf("--------------------------------------\n");

    // h.i.распечатывает ее значение;
    char *value = getenv(env_var);
    printf("начальное значение: %s\n", value);

    // h.ii. изменяет его значение;
    setenv(env_var, "bbb", 1);
    printf("! значение переменной изменено !\n");

    // h.iii. повторно распечатывает ее значение.
    value = getenv(env_var);
    printf("новое значение: %s\n", value);

    printf("--------------------------------------\n");
    system("echo $MY_ENV_VAR");

    return 0;
}