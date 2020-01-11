all: osx
	- out/cturing_osx < tm_description.txt

osx:
	- mkdir -p out
	- gcc -o out/cturing_osx cturing.c

linux:
	- mkdir -p out
	- gcc -o out/cturing_linux cturing.c

windows:
	- mkdir -p out
	- i686-w64-mingw32-gcc -o out/cturing.exe cturing.c