#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int Task(const char *text, const char *str) {
    size_t n = strlen(text);
    size_t m = strlen(str);
    if (m == 0) {
        return 0;
    }
    if (n < m) {
        return -1;
    }

    for (size_t i = 0; i <= n - m; ++i) {
        size_t j;
        for (j = 0; j < m; ++j) {
            if (text[i + j] != str[j]) {
                break;
            }
        }
        if (j == m) {
            return (int)i;
        }
    }
    return -1;
}

double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}


int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Строго по примеру: %s <файл с текстом> <подстрока>\n", argv[0]);
        return 1;
    }

    char* str = argv[2];

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Ошибка с открытием файла\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* text = malloc((size_t)size + 1);
    fread(text, 1, size, file);
    text[size] = '\0';
    fclose(file);

    printf("Последовательный:\n");
    printf("Длина текста: %zu\n", strlen(text));
    printf("Длина образца: %zu\n", strlen(str));

    double t0 = GetTime();
    int result = Task(text, str);
    double t = GetTime() - t0;

    printf("Результат: %d\n", result);
    printf("Время: %.3f\n", t);

    free(text);
    return 0;
}
