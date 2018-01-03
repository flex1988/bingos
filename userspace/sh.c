#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <types.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

void sh_loop();
char *sh_read_line(char *buffer);
int sh_split_line(char *line, char **tokens, int *tlen);
int sh_execute(int argc, char **argv);

void sh_loop() {
    char *line;
    char *args;
    char *tokens = malloc(LSH_TOK_BUFSIZE);
    char *buffer = malloc(LSH_RL_BUFSIZE);
    
    int status = 0;

    int tlen;

    do {
        memset(tokens,0x0,LSH_TOK_BUFSIZE);
        printf("> ");
        line = sh_read_line(buffer);
        sh_split_line(line, tokens, &tlen);
        status = sh_execute(tlen, tokens);
    } while (status);

    free(tokens);
    free(buffer);
}

int sh_execute(int argc, char **argv) {
    int pid;
    int ret;
    if ((pid = fork()) == 0) {
        ret = execvp(argv[0], argv);
        if (ret == -1)
            exit(0);
    } else {
        int s = waitpid(pid);
        return 1;
    }
}

char *sh_read_line(char *buffer) {
    int position = 0;

    char c;

    if (!buffer)
        exit(-1);

    while (1) {
        read(0, &c, 1);

        if (c == '\r' || c == '\n') {
            buffer[position] = '\0';
            printc('\n');
            position++;
            return buffer;
        } else if (c == 0x08) {
            if (position) {
                printc(c);
                position--;
            }
        } else {
            printc(c);
            buffer[position] = c;
            position++;
        }
    }
}

int sh_split_line(char *line, char **tokens, int *tlen) {
    char *last, *token;
    *tlen = 0;

    for (token = strtok_r(line, " ", &last); token; token = strtok_r(NULL, " ", &last)) {
        tokens[(*tlen)++] = token;
    }

    return 0;
}

int main(int argc, char **argv) {
    sh_loop();

    return 0;
}
