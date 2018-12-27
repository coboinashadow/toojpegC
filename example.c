#include "toojpeg.h"
#include <stdio.h>

FILE* outfile;

// Callback
void writeJpegCallback(unsigned char byte) {
	fputc(byte, outfile);
}

int main() {
	int width = 500;
	int height = 500;
	int radius = 200;

	// Pixels buffer (500x500 RGB)
	unsigned char pixels[width * height * 3];

	// Fill with grey (80, 80, 80)
	for(int i = 0; i < width * height * 3; i++) pixels[i] = 80;

	// Draw blue square
	for(int y = 300; y < 400; y++) {
		for(int x = 300; x < 400; x++) {
			int c = (y * width + x) * 3; //posicao do pixel no bitmap
			pixels[c] = 50;		//vermelho
			pixels[c + 1] = 130;	//verde
			pixels[c + 2] = 190;	//azul
		}
	}

	// Save to drive
	outfile = fopen("image.jpeg", "wb");
	// JPEG encode
	writeJpeg(writeJpegCallback, pixels, width, height);

	fclose(outfile);
	printf("Image JPEG created: image.jpeg\n");
	return 0;
}
