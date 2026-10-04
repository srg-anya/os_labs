#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char out1[256];
    char out2[256];

    int n1 = read(0, out1, 255);
    int n2 = read(0, out2, 255);
    if (n1 <= 0 || n2 <= 0) {
        write(2, "Ошибка ввода\n", 13);
        exit(-1);
    }
    out1[n1 - 1] = '\0';
    out2[n2 - 1] = '\0';


    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        write(2, "Ошибка создания pipe\n", 20);
        exit(-1);
    }

    const pid_t child1 = fork();
    if (child1 == -1) {
        write(2, "Ошибка fork\n", 11);
        exit(-1);
    } else if (child1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        execlp("./child1", "child1", out1, NULL);
        write(2, "Ошибка запуска child1\n", 22);
        exit(-1);
    }

    const pid_t child2 = fork();
    if (child2 == -1) {
        write(2, "Ошибка fork\n", 11);
        exit(-1);
    } else if (child2== 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        execlp("./child2", "child2", out2, NULL);
        write(2, "Ошибка запуска child2\n", 22);
        exit(-1);
    }

    close(pipe1[0]);
    close(pipe2[0]);

    char buffer[2048];
    char line[1024];
    int ind = 0;
    int lineNum = 1;
    int bytes;
    while (1) {
        bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes == -1) {
            write(2, "Ошибка чтения\n", 15);
            exit(-1);
        }
        if (bytes == 0) {
            break;
        }
        for (int i = 0; i < bytes; ++i) {
            char c = buffer[i];
            if (c == '\n') {
                line[ind] = '\0';
                if (lineNum % 2 != 0) {
                    write(pipe1[1], line, ind);
                    write(pipe1[1], "\n", 1);
                } else {
                    write(pipe2[1], line, ind);
                    write(pipe2[1], "\n", 1);
                }
                ++lineNum;
                ind = 0;
            } else {
                if (ind < sizeof(line) - 1) {
                    line[ind++] = c;
                }
            }
        }
    }
    if (ind > 0) {
        line[ind] = '\0';
        if (lineNum % 2 != 0) {
            write(pipe1[1], line, ind);
            write(pipe1[1], "\n", 1);
        } else {
            write(pipe2[1], line, ind);
            write(pipe2[1], "\n", 1);
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    return 0;
}