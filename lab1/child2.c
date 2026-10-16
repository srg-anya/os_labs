#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int IsG(char c) {
    if (c >= 'A' && c <= 'Z') {
        c += 32;
    }
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y' ||
        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U' || c == 'Y') {
        return 1;
    }
    return 0;
}

int Result(char* s, char* res, int len) {
    int k = 0;
    for (int i = 0; i < len; ++i) {
        if (!IsG(s[i])) {
            res[k] = s[i];
            ++k;
        }
    }
    res[k] = '\0';
    return k;
}

int main() {
    char buffer[1024];
    char result[1024];
    int bytes;
    while (1) {
        bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes == -1) {
            write(2, "Ошибка чтения\n", 26);
            exit(-1);
        } else if (bytes == 0) {
            break;
        }
        int len = Result(buffer, result, bytes);
        if (write(STDOUT_FILENO, result, len) == -1) {
            write(2, "Ошибка записи\n", 26);
            exit(-1);
        }
    }
    return 0;
}