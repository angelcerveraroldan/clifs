# Configure
conf:
	cmake -S . -B build -G Ninja

# Compile
build:
	cmake --build build

# Compile
all: build

# Configure, compile, and run
run: all
	./build/clifs

# Clean build
clean:
	rm -rf build
