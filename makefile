build:
	gcc -Wall -I src/include -L src/lib -o Teste main.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
run:
	./teste