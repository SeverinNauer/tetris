.PHONY: all build run clean rebuild

all: build

build:
	cmake -B build && cmake --build build && ln -sf build/compile_commands.json .

run: build
	nixGL ./build/raylib-demo

clean:
	rm -rf build

rebuild: clean build
