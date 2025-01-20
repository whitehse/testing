all: build_fresh

arm: build_fresh_arm

build:
	( cd build && make )

build_fresh:
	rm -rf build
	mkdir build
	( cd build && cmake .. && cmake --build . )

# sudo apt install gcc-arm-linux-gnueabihf
build_fresh_arm:
	rm -rf build
	mkdir build
	#( cd build && cmake -DCMAKE_BUILD_TYPE=Release -DARM=ON .. && cmake --build . )
	( cd build && cmake -DCMAKE_BUILD_TYPE=Release -DARM=ON -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc-12 .. && cmake --build . )

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

