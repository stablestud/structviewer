build: bin compile

bin:
	mkdir --parent bin
compile:
	g++ -o bin/structviewer src/*.cpp -lwtsapi32 -lkernel32 -ladvapi32 -static -static-libgcc -static-libstdc++
installdir:
	mkdir -p /usr/local/bin
install: build installdir
	cp ./bin/structviewer.exe /usr/local/bin/


.PHONY: build compile installdir install
