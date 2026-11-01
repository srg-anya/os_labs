#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    const char* text;
    const char* str;
    size_t start;
    size_t ind;
    size_t lenText;
    size_t lenStr;
} ThreadArgs;

static int globalFound = -1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int Task(const char *text, const char *str, size_t start, size_t end, size_t lenText, size_t lenStr) {
    if (lenStr == 0) {
        return 0;
    }
    if (lenText < lenStr) {
        return -1;
    }

    for (size_t i = start; i <= end; ++i) {
        size_t j;
        for (j = 0; j < lenStr; ++j) {
            if (text[i + j] != str[j]) {
                break;
            }
        }
        if (j == lenStr) {
            return (int)i;
        }
    }
    return -1;
}


static void* work(void *_args) {
    ThreadArgs* args = (ThreadArgs*)_args;

    int localFound = Task(args->text, args->str, args->start, args->ind, args->lenText, args->lenStr);

    pthread_mutex_lock(&mutex);
    if (localFound != -1) {
        if (globalFound == -1) {
            globalFound = localFound;
        } else if (localFound < globalFound) {
            globalFound = localFound;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}


double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}


int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Строго по примеру: %s <файл с текстом> <подстрока> <число потоков>\n", argv[0]);
        return 1;
    }

    char* str = argv[2];
    int nThreads = atoi(argv[3]);
    if (nThreads < 1) {
        nThreads = 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Ошибка с открытием файла\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* text = malloc((size_t)size + 1);
    fread(text, 1, (size_t)size, file);
    text[size] = '\0';
    fclose(file);

    size_t lenText = strlen(text);
    size_t lenStr = strlen(str);

    printf("Параллельный:\n");
    printf("Длина текста: %zu\n", lenText);
    printf("Длина образца: %zu\n", lenStr);
    printf("Число потоков: %d\n", nThreads);

    size_t totalPositions = lenText - lenStr + 1;
    if ((size_t)nThreads > totalPositions) {
        nThreads = (int)totalPositions;
    }

    pthread_t* threads = malloc((size_t)nThreads * sizeof(pthread_t));
    ThreadArgs* threadArgs = malloc((size_t)nThreads * sizeof(ThreadArgs));

    size_t start = 0;
    size_t base = totalPositions / (size_t)nThreads;
    size_t rem = totalPositions % (size_t)nThreads;

    double t0 = GetTime();

    for (int i = 0; i < nThreads; ++i) {
        size_t block = base;
        if ((size_t)i < rem) {
            block = block + 1;
        }

        threadArgs[i] = (ThreadArgs){
            .text = text,
            .str = str,
            .start = start,
            .ind = start + block - 1,
            .lenText = lenText,
            .lenStr = lenStr
        };
        start += block;

        pthread_create(&threads[i], NULL, work, &threadArgs[i]);
    }

    for (int i = 0; i < nThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    double t = GetTime() - t0;

    printf("Результат: %d\n", globalFound);
    printf("Время: %.3f\n", t);

    free(threadArgs);
    free(threads);
    free(text);
    return 0;
}