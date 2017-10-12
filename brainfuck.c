#include <stdio.h>
#include <stdlib.h>

#define MAX_LOOP 1000
#define TAPE_LENGTH 300000

#define error(msg) exit(fprintf(stderr, "%s\n", msg))

void execute(char *program) {
    char tape[TAPE_LENGTH];
    int stack[MAX_LOOP];
    int sp = 0;
    int ip = 0;
    int dp = 0;

    int n;
    int skip = 0;

    for (;;) {
        char i = program[ip++];
        switch (i) {
            case '>':
                n = 1;
                while (program[ip] == '>') {
                    n++;
                    ip++;
                }
                if (!skip) {
                    dp += n;
                    while (dp >= TAPE_LENGTH)
                        dp -= TAPE_LENGTH;
                }
                break;

            case '<':
                n = 1;
                while (program[ip] == '<') {
                    n++;
                    ip++;
                }
                if (!skip) {
                    dp -= n;
                    while (dp < 0)
                        dp += TAPE_LENGTH;
                }
                break;

            case '+':
                n = 1;
                while (program[ip] == '+') {
                    n++;
                    ip++;
                }
                if (!skip) tape[dp] += n;
                break;

            case '-':
                n = 1;
                while (program[ip] == '-') {
                    n++;
                    ip++;
                }
                if (!skip) tape[dp] -= n;
                break;

            case ',':
                if (!skip) {
                    tape[dp] = (char)getchar();
                }
                break;

            case '.':
                if (!skip) {
                    putchar((int)tape[dp]);
                }
                break;

            case '[':
                stack[sp++] = ip;
                if (sp >= MAX_LOOP) error("too deep recursion");
                if (!tape[dp]) skip ++;
                break;

            case ']':
                if (sp <= 0) error("unmatched ']'");

                if (tape[dp]) ip = stack[sp-1];
                else sp --;
               
                if (skip) skip --;
                break;

            case 0:
                if (sp > 0) error("unmatched '[]");
                return;
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 2) {
        long sz;
        char *program;
        FILE *f = fopen(argv[1], "r");
        if (!f) error("can't open file");

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        program = (char*) malloc(sz+1);
        program[fread(program, 1, sz, f)] = 0;
        fclose(f);
        execute(program);
    } else {
        error("usage: brainfuck bffile");
    }

    return 0;
}
