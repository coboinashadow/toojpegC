#include "toojpeg.h"
#include <stdio.h>

FILE* outfile;

// Callback para escrever dados
void writeJpegCallback(unsigned char byte) {
	fputc(byte, outfile);
}

int main() {
	int width = 500;
	int height = 500;
	int radius = 200;
	int cx = 250; //centro do circulo
	int cy = 250; //centro do circulo

	// Criar buffer de pixels (500x500 RGB)
	unsigned char pixels[width * height * 3];

	// Preencher com cinza (80, 80, 80)
	for(int i = 0; i < width * height * 3; i++) pixels[i] = 80;
	// Draw big circle
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int dx = x - cx;
			int dy = y - cy;
			//se o pixel esta dentro do circulo
			if(dx * dx + dy * dy <= radius * radius) {
				int c = (y * width + x) * 3; //posicao do pixel no bitmap
				pixels[c] = 50;		//vermelho
				pixels[c + 1] = 130;	//verde
				pixels[c + 2] = 190;	//azul
			}
		}
	}

	radius = 180;
	// Draw inner circle
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int dx = x - cx;
			int dy = y - cy;
			//se o pixel esta dentro do circulo
			if(dx * dx + dy * dy <= radius * radius) {
				int c = (y * width + x) * 3; //posicao do pixel no bitmap
				pixels[c] = 30;		//vermelho
				pixels[c + 1] = 120;	//verde
				pixels[c + 2] = 80;	//azul
			}
		}
	}

	// Abrir arquivo para escrita
	outfile = fopen("image.jpeg", "wb");
	if (!outfile)
		printf("Erro ao criar arquivo\n");

	// Codificar como JPEG
	writeJpeg(writeJpegCallback, pixels, width, height);

	fclose(outfile);

	printf("Image JPEG created: image.jpeg\n");

	return 0;
}
