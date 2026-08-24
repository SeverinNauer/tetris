.PHONY: all build run clean rebuild web

all: build

build:
	cmake -B build -G Ninja
	cmake --build build
	ln -sf build/compile_commands.json .

run: build
	nixGL ./build/raylib-demo

web:
	emcmake cmake -B build/web -DCMAKE_BUILD_TYPE=Release -G Ninja
	cmake --build build/web

clean:
	rm -rf build

rebuild: clean build
