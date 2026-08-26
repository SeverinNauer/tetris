.PHONY: all build run clean rebuild web deps

all: build

build:
	cmake -B build -DUSE_EXTERNAL_GLFW=ON && cmake --build build && ln -sf build/compile_commands.json .

run: build
	nixGL ./build/raylib-demo

deps:
	@if [ ! -d deps/raylib-src ]; then git clone --depth 1 --branch 6.0 https://github.com/raysan5/raylib.git deps/raylib-src; fi
	$(MAKE) -C deps/raylib-src/src PLATFORM=PLATFORM_WEB -B
	@mkdir -p deps/raylib-web/include deps/raylib-web/lib
	@cp deps/raylib-src/src/raylib.h deps/raylib-src/src/raymath.h deps/raylib-src/src/rlgl.h deps/raylib-web/include/
	@cp deps/raylib-src/src/libraylib.web.a deps/raylib-web/lib/

web: deps
	mkdir -p build/web
	emcc -o build/web/index.html src/main.c -O3 \
	  -Wall -std=c23 -D_DEFAULT_SOURCE -DPLATFORM_WEB \
	  -I deps/raylib-web/include \
	  deps/raylib-web/lib/libraylib.web.a \
	  -sUSE_GLFW=3 \
	  -sASYNCIFY \
	  -sALLOW_MEMORY_GROWTH=1 -sGROWABLE_ARRAYBUFFERS=0 -sINITIAL_MEMORY=134217728 \
	  -sFORCE_FILESYSTEM=1 \
	  -sEXPORTED_RUNTIME_METHODS=ccall \
	  -sMINIFY_HTML=0 \
	  --shell-file web/shell.html \
	  --preload-file resources

clean:
	rm -rf build

rebuild: clean build
