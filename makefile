all: feu generation display

feu: feu.c
	gcc -o feu feu.c -lpthread
	
generation: car_generation.c
	gcc -o car_generation car_generation.c
	
display: display.c
	gcc -o display display.c -lpthread
	
clean:
	rm -f feu car_generation display
