from array import array
from sys import stdin, stdout

class BrainfuckVM(object):
    def __init__(self):
        pass

    def execute(self, program):
        TAPE_SIZE = 30000
        data = bytearray(TAPE_SIZE)
        dp = ip = 0
        stack = array('I')
        skip = 0

        while True:
            i = program[ip]
            ip += 1
            if i == '<':
                n = 1
                while program[ip] == '<':
                    ip += 1
                    n += 1
                if skip == 0:
                    dp -= n
                    while dp < 0:
                        dp += TAPE_SIZE
            elif i == '>':
                n = 1
                while program[ip] == '>':
                    ip += 1
                    n += 1
                if skip == 0:
                    dp += n
                    dp = dp % TAPE_SIZE
            elif i == '+':
                n = 1
                while program[ip] == '+':
                    ip += 1
                    n += 1
                if skip == 0:
                    data[dp] = (data[dp] + n) % 256
            elif i == '-':
                n = 1
                while program[ip] == '-':
                    ip += 1
                    n += 1
                if skip == 0:
                    r = data[dp] - n
                    while r < 0:
                        r += 256
                    data[dp] = r
            elif i == ',':
                if skip == 0:
                    data[dp] = ord(stdin.read(1))
            elif i == '.':
                if skip == 0:
                    stdout.write(chr(data[dp]))
            elif i == '[':
                stack.append(ip)
                if data[dp] == 0:
                    skip += 1
            elif i == ']':
                if len(stack) == 0:
                    raise ValueError("no corresponding '['")
                if data[dp] == 0:
                    stack.pop()
                else:
                    ip = stack[-1]
                if skip > 0:
                    skip -= 1
            elif i == '\0':
                if len(stack) > 0:
                    raise ValueError("no corresponding ']'")
                return

if __name__ == '__main__':
    from sys import argv
    if len(argv) != 2:
        print('Usage: brainfuck bffile')
    else:
        with open(argv[1]) as bf:
            program = bf.read()

        program = program + '\0'
        vm = BrainfuckVM()
        vm.execute(program)
