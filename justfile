default:
    @ just --list

clean-debug:
    @ rm -rf build/debug

clean-release:
    @ rm -rf build/release

clean: clean-debug clean-release

configure-debug:
    cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
    @ ln -sf build/debug/compile_commands.json compile_commands.json

configure-release:
    cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release

configure: configure-debug configure-release

build-debug: configure-debug
    @ ninja -C build/debug

build-release: configure-release
    @ ninja -C build/release

build: build-debug build-release

unittests: build-debug
    @ ./build/debug/unittests

systemtests: build-debug
    @ ./build/debug/systemtests

test: unittests systemtests

format:
    @ clang-format-19 -i --style=file --verbose src/*.cpp src/**/*.cpp src/*.h src/**/*.h unittests/*.cpp systemtests/*.cpp

all: format build test
    @echo "✅ All tasks completed successfully!"

dev: build unittests
    @echo "✅ Development cycle completed!"

bench: build-release
    /usr/lib/linux-tools-6.8.0-88/perf record -F 997 --call-graph dwarf ./build/release/endomorphism
    /usr/lib/linux-tools-6.8.0-88/perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
