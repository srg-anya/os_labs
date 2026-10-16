#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static const char CHILD1_NAME[] = "child1";
static const char CHILD2_NAME[] = "child2";

static int join_path(char *buf, int size, const char *dir, const char *name) {
    int k = 0;

    for (int i = 0; dir[i] && k < size - 1; ++i) {
        buf[k++] = dir[i];
    }

    if (k < size - 1) {
        buf[k++] = '/';
    }

    for (int i = 0; name[i] && k < size - 1; ++i) {
        buf[k++] = name[i];
    }

    buf[k] = '\0';
    return k;
}

int main() {
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

    char progpath[1024];
    ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
    if (len == -1) {
        write(2, "Ошибка readlink\n", 22);
        exit(-1);
    }
    progpath[len] = '\0';

    while (len > 0 && progpath[len] != '/') {
        --len;
    }
    if (len == 0) {
        write(2, "Ошибка разбора пути\n", 37);
        exit(-1);
    }
    progpath[len] = '\0';

    char path1[1024];
    char path2[1024];
    join_path(path1, sizeof(path1), progpath, CHILD1_NAME);
    join_path(path2, sizeof(path2), progpath, CHILD2_NAME);

    int fd1 = open(out1, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd1 == -1) {
        write(STDERR_FILENO, "open file1 error\n", 17);
        exit(-1);
    }

    int fd2 = open(out2, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd2 == -1) {
        write(STDERR_FILENO, "open file2 error\n", 17);
        exit(-1);
    }

    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        write(2, "Ошибка создания pipe\n", 35);
        exit(-1);
    }

    const pid_t child1 = fork();
    if (child1 == -1) {
        write(2, "Ошибка fork\n", 18);
        exit(-1);
    } else if (child1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        dup2(fd1, STDOUT_FILENO);
        close(fd1);

        char *const args[] = {(char *)CHILD1_NAME, NULL};
        execv(path1, args);
        write(2, "Ошибка запуска child1\n", 35);
        exit(-1);
    }

    const pid_t child2 = fork();
    if (child2 == -1) {
        write(2, "Ошибка fork\n", 18);
        exit(-1);
    } else if (child2== 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        dup2(fd2, STDOUT_FILENO);
        close(fd2);

        char *const args[] = {(char *)CHILD2_NAME, NULL};
        execv(path2, args);
        write(2, "Ошибка запуска child2\n", 35);
        exit(-1);
    }

    close(pipe1[0]);
    close(pipe2[0]);
    close(fd1);
    close(fd2);

    char buffer[2048];
    char line[1024];
    int ind = 0;
    int lineNum = 1;
    int bytes;
    while (1) {
        bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes == -1) {
            write(2, "Ошибка чтения\n", 26);
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