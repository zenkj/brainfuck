def translate(program):
    py = []
    ip = 0
    indent = 0

    INDENT = '  '

    def put(line):
        py.append(INDENT*indent + line)

    put('from sys import stdin, stdout')
    put('TAPE_SIZE = 30000')
    put('data = bytearray(TAPE_SIZE)')
    put('dp = 0')

    while True:
        i = program[ip]
        ip += 1
        if i == '<':
            n = 1
            while program[ip] == '<':
                ip += 1
                n += 1
            put('dp -= %d%%TAPE_SIZE' % n)
            put('dp = dp + TAPE_SIZE if dp < 0 else dp')
        elif i == '>':
            n = 1
            while program[ip] == '>':
                ip += 1
                n += 1
            put('dp += %d%%TAPE_SIZE' % n)
            put('dp = dp - TAPE_SIZE if dp >= TAPE_SIZE else dp')
        elif i == '+':
            n = 1
            while program[ip] == '+':
                ip += 1
                n += 1
            put('data[dp] = (data[dp] + %d) %% 256' % n)
        elif i == '-':
            n = 1
            while program[ip] == '-':
                ip += 1
                n += 1
            put('r = data[dp] - %d' % n)
            put('data[dp] = r if r >= 0 else r + 256')
        elif i == ',':
            put('data[dp] = ord(stdin.read(1))')
        elif i == '.':
            put('stdout.write(chr(data[dp]))')
        elif i == '[':
            put('while data[dp] != 0:')
            indent += 1
        elif i == ']':
            if indent <= 0:
                raise ValueError("no corresponding '['")
            indent -= 1
        elif i == '\0':
            if indent > 0:
                raise ValueError("no corresponding ']'")
            return '\n'.join(py)

if __name__ == '__main__':
    from sys import argv
    if len(argv) != 2:
        print('Usage: python brainfuck-to-py.py bffile')
    else:
        with open(argv[1]) as bf:
            program = bf.read()

        program = program + '\0'

        pyprogram = translate(program)

        with open(argv[1]+'.py', 'w') as pybf:
            pybf.write(pyprogram)
