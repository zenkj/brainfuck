time: all
	bin/brainfuck-jit mandelbrot.bf | tee mandelbrot.time
	echo brainfuck ... >> mandelbrot.time
	/usr/bin/time -pao mandelbrot.time bin/brainfuck mandelbrot.bf
	echo brainfuck-O3 ... >> mandelbrot.time
	/usr/bin/time -pao mandelbrot.time bin/brainfuck-O3 mandelbrot.bf
	echo brainfuck-jit ... >> mandelbrot.time
	/usr/bin/time -pao mandelbrot.time bin/brainfuck-jit mandelbrot.bf
	echo brainfuck.py ... >> mandelbrot.time
	/usr/bin/time -pao mandelbrot.time python brainfuck.py mandelbrot.bf
	echo brainfuck-jit.py ... >> mandelbrot.time
	/usr/bin/time -pao mandelbrot.time python mandelbrot.bf.py

bin/brainfuck-jit: brainfuck-jit.c
	bin/luajit dynasm/dynasm.lua -o brainfuck-jit-x64.c -D X64 $^
	gcc -g brainfuck-jit-x64.c -o$@

bin/brainfuck: brainfuck.c
	gcc -g $^ -o$@

bin/brainfuck-O3: brainfuck.c
	gcc -O3 -g $^ -o$@

mandelbrot.bf.py: mandelbrot.bf
	python brainfuck-to-py.py mandelbrot.bf

all: bin/brainfuck bin/brainfuck-jit bin/brainfuck-O3 mandelbrot.bf.py

clean:
	rm -f brainfuck-jit-x64.c bin/brainfuck bin/brainfuck-jit bin/brainfuck-O3 mandelbrot.bf.py
