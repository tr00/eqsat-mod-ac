default:
    @ just --list

clean:
    @ ninja -C build -t clean

configure:
    cmake -B build -G Ninja

build:
    @ ninja -C build

test: build
    ninja -C build runtest

unittests: build
    @ ./build/unittests

systemtests: build
    @ ./build/systemtests

format:
    @ clang-format-19 -i --style=file --verbose src/*.cpp src/**/*.cpp src/*.h src/**/*.h unittests/*.cpp systemtests/*.cpp

all: format build test
    @echo "✅ All tasks completed successfully!"

dev: build unittests
    @echo "✅ Development cycle completed!"
