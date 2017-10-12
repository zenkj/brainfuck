#include "dynasm/dasm_proto.h"
#include "dynasm/dasm_x86.h"
#include <stdio.h>
#include <stdlib.h>
#if _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

#define MAX_LOOP 1000
#define TAPE_LENGTH 300000

#define error(msg) exit(fprintf(stderr, "%s\n", msg))

static void *link_and_encode(dasm_State** d) {
    size_t sz;
    void* buf;
    dasm_link(d, &sz);

#ifdef _WIN32
    buf = VirtualAlloc(0, sz, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#else
    buf = mmap(0, sz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif

    dasm_encode(d, buf);

#ifdef _WIN32
    DWORD dwOld;
    VirtualProtect(buf, sz, PAGE_EXECUTE_READ, &dwOld);
#else
    mprotect(buf, sz, PROT_READ|PROT_EXEC);
#endif

    return buf;
}

typedef struct bf_state {
    char *tape;
    char (*get_char)();
    void (*put_char)(char);
} bf_state;

//void execute(char *program) {
void (*compile(char *program))(bf_state *state) {
    int stack[MAX_LOOP];
    int sptr = 0;

    int ip = 0;

    dasm_State *d;
    unsigned int npc = 8;
    unsigned int nextpc = 0;

    int n;

    |.if X64
    |.arch x64
    |.else
    |.arch x86
    |.endif

    |.section code
    dasm_init(&d, DASM_MAXSECTION);

    |.globals lbl_
    void* labels[lbl__MAX];
    dasm_setupglobal(&d, labels, lbl__MAX);

    |.actionlist bf_actions
    dasm_setup(&d, bf_actions);

    dasm_growpc(&d, npc);

    |.if X64
      |.define aPtr, rbx
      |.define aState, r12
      |.if WIN
        |.define aTapeBegin, rsi
        |.define aTapeEnd, rdi
        |.define rArg1, rcx
        |.define rArg2, rdx
      |.else
        |.define aTapeBegin, r13
        |.define aTapeEnd, r14
        |.define rArg1, rdi
        |.define rArg2, rsi
      |.endif
      |.macro precall1, arg1
        | mov rArg1, arg1
      |.endmacro
      |.macro precall2, arg1, arg2
        | mov rArg1, arg1
        | mov rArg2, arg2
      |.endmacro
      |.define postcall, .nop
      |.macro prologue
        | push aPtr
        | push aState
        | push aTapeBegin
        | push aTapeEnd
        | push rax
        | mov aState, rArg1
      |.endmacro
      |.macro epilogue
        | pop rax
        | pop aTapeEnd
        | pop aTapeBegin
        | pop aState
        | pop aPtr
        | ret
      |.endmacro
    |.else
      |.define aPtr, ebx
      |.define aState, ebp
      |.define aTapeBegin, esi
      |.define aTapeEnd, edi
      |.macro precall1, arg1
        | push arg1
      |.endmacro
      |.macro precall2, arg1, arg2
        | push arg1
        | push arg2
      |.endmacro
      |.macro postcall, n
        | add esp, 4*n
      |.endmacro
      |.macro prologue
        | push aPtr
        | push aState
        | push aTapeBegin
        | push aTapeEnd
        | mov aState, [esp+20]
      |.endmacro
      |.macro epilogue
        | pop aTapeEnd
        | pop aTapeBegin
        | pop aState
        | pop aPtr
        | ret 4
      |.endmacro
    |.endif

    |.type state, bf_state, aState

    dasm_State ** Dst = &d;

    |.code
    |->bf_main:
    | prologue
    | mov aPtr, state->tape
    | lea aTapeBegin, [aPtr-1]
    | lea aTapeEnd, [aPtr+TAPE_LENGTH-1]

    for (;;) {
        char i = program[ip++];
        switch (i) {
            case '>':
                n = 1;
                while (program[ip] == '>') {
                    n++;
                    ip++;
                }
                | add aPtr, n%TAPE_LENGTH
                | cmp aPtr, aTapeEnd
                | jbe >1
                | sub aPtr, TAPE_LENGTH
                |1:
                break;

            case '<':
                n = 1;
                while (program[ip] == '<') {
                    n++;
                    ip++;
                }
                | sub aPtr, n%TAPE_LENGTH
                | cmp aPtr, aTapeBegin
                | ja >1
                | add aPtr, TAPE_LENGTH
                |1:
                break;

            case '+':
                n = 1;
                while (program[ip] == '+') {
                    n++;
                    ip++;
                }
                | add byte [aPtr], n
                break;

            case '-':
                n = 1;
                while (program[ip] == '-') {
                    n++;
                    ip++;
                }
                | sub byte [aPtr], n
                break;

            case ',':
                | call aword state->get_char
                | postcall 1
                | mov byte [aPtr], al
                break;

            case '.':
                | movzx r0, byte [aPtr]
                | precall1 r0
                |call aword state->put_char
                | postcall 2
                break;

            case '[':
                if (sptr == MAX_LOOP)
                    error("Nesting too deep");
                if (program[ip] == '-' && program[ip+1] == ']') {
                    ip += 2;
                    | xor eax, eax
                    | mov byte [aPtr], al
                } else {
                    if (nextpc == npc) {
                        npc *= 2;
                        dasm_growpc(&d, npc);
                    }
                    | cmp byte [aPtr], 0
                    | jz =>nextpc + 1
                    |=>nextpc:
                    stack[sptr++] = nextpc;
                    nextpc += 2;
                }         
                break;

            case ']':
                if (sptr <= 0) error("unmatched ']'");

                --sptr;
                | cmp byte [aPtr], 0
                | jnz =>stack[sptr]
                |=>stack[sptr]+1:
                break;

            case 0:
                if (sptr > 0) error("unmatched '[]");
                | epilogue
                link_and_encode(&d);
                dasm_free(&d);
                return (void(*)(bf_state*))labels[lbl_bf_main];
        }
    }
}

char get_char() {
    return (char)getchar();
}

void put_char(char ch) {
    putchar((int)ch);
}

int main(int argc, char** argv) {
    if (argc == 2) {
        long sz;
        char *program;
        char tape[TAPE_LENGTH];
        bf_state state;
        state.tape = tape;
        state.get_char = get_char;
        state.put_char = put_char;

        FILE *f = fopen(argv[1], "r");
        if (!f) error("can't open file");

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        program = (char*) malloc(sz+1);
        program[fread(program, 1, sz, f)] = 0;
        fclose(f);
        compile(program)(&state);
    } else {
        error("usage: brainfuck bffile");
    }

    return 0;
}
