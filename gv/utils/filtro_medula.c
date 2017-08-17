#define COLOR_FONDO 200
#define COLOR_BAJO 40

static int vecino_es_fondo(int x, int y, int xsize, int ysize, double *image) {
	if ((x<=0) || (x>=xsize-1)) return 1;
	if ((y<=0) || (y>=ysize-1)) return 1;

	/* verificamos los 8 vecinos buscando fondo*/
	if (image[y*xsize+(x-1)] >= COLOR_FONDO) return 1;
	if (image[y*xsize+(x+1)] >= COLOR_FONDO) return 1;
	if (image[(y-1)*xsize+(x-1)] >= COLOR_FONDO) return 1;
	if (image[(y-1)*xsize+(x)] >= COLOR_FONDO) return 1;
	if (image[(y-1)*xsize+(x+1)] >= COLOR_FONDO) return 1;
	if (image[(y+1)*xsize+(x-1)] >= COLOR_FONDO) return 1;
	if (image[(y+1)*xsize+(x)] >= COLOR_FONDO) return 1;
	if (image[(y+1)*xsize+(x+1)] >= COLOR_FONDO) return 1;

	return 0;
}


void filtro_medula(int xsize, int ysize, double *image) {
	int x, y;

	for (y=0;y<ysize;y++) {
    	for (x=0;x<xsize;x++) {
		    if ((image[y*xsize+x] < 150) && 
		        (! vecino_es_fondo(x, y, xsize, ysize, image)) &&
		        (! vecino_es_fondo(x-1, y-1, xsize, ysize, image)) &&
		        (! vecino_es_fondo(x-1, y+1, xsize, ysize, image)) &&
		        (! vecino_es_fondo(x+1, y-1, xsize, ysize, image)) &&
		        (! vecino_es_fondo(x+1, y-1, xsize, ysize, image)) )
			    image[y*xsize+x] = 0;
	    }
    }
}

