# Configure
conf:
	cmake -S . -B build -G Ninja

# Compile
build: conf
	cmake --build build --target clifs --verbose

# Configure, compile, and run
run: build
	./build/clifs

# Clean build
clean:
	rm -rf build
