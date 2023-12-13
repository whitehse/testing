all: build_fresh

wasm: build_fresh_wasm

build:
	( cd build && make )

build_fresh:
	rm -rf build
	mkdir build
	( cd build && cmake \
    -DMZ_ZLIB=ON \
    -DMZ_BZIP2=OFF \
    -DMZ_LZMA=OFF \
    -DMZ_ZSTD=OFF \
    -DMZ_OPENSSL=OFF \
    -DMZ_PKCRYPT=OFF \
    -DMZ_WZAES=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DEXPAT_BUILD_TESTS=OFF \
    -DEXPAT_BUILD_TOOLS=OFF \
    -DEXPAT_SHARED_LIBS=OFF \
    -DEXPAT_WITH_SYS_GETRANDOM=OFF \
    .. && cmake --build . )

build_fresh_wasm:
	rm -rf build
	mkdir build
	( cd build && cmake \
    -DWASM=ON \
    -DCMAKE_C_COMPILER=/opt/wasi-sdk/bin/clang \
    -DMZ_ZLIB=ON \
    -DMZ_BZIP2=ON \
    -DMZ_LZMA=OFF \
    -DMZ_ZSTD=OFF \
    -DMZ_OPENSSL=OFF \
    -DMZ_PKCRYPT=ON \
    -DMZ_WZAES=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DEXPAT_BUILD_TESTS=OFF \
    -DEXPAT_BUILD_TOOLS=OFF \
    -DEXPAT_SHARED_LIBS=OFF \
    -DEXPAT_WITH_SYS_GETRANDOM=OFF \
    .. && cmake --build . )

clean:
	cd build && make clean

install_min:
	sudo apt-get install gcc g++ cmake make doxygen

install_tests: install_min
	sudo apt-get install gcovr lcov

install: install_min install_tests
	sudo apt-get install git llvm pkg-config curl zip unzip tar python3-dev clang-format clang-tidy

install_pip:
	pip install jinja2 Pygments cmake-format pre-commit

install_doc: install_min
	sudo apt-get install doxygen
	pip install jinja2 Pygments

prepare:
	rm -rf build
	mkdir build

graphviz:
	cd build && cmake .. --graphviz=graph.dot && dot -Tpng graph.dot -o graph.png

docs:
	cd build && cmake --build . --target=docs

test:
	cd build && ctest
