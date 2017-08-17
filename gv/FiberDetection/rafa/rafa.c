#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void calcular_varianza_desvio_estandar(double * medidas, int sum, int cant, int mi);

static double medidas[200]; // cada medida encontrada, util para luego sacar la variancia y la desviacion estandar

static int es_fondo(double x, double y, double ix, double iy, double *image, int xsize, int ysize) {
	int r=0;
	unsigned int mx, my;
	unsigned int ux, uy, uix, uiy;


	ux = (unsigned int) x;
	uy = (unsigned int) y;
	uix = (unsigned int) ix;
	uiy = (unsigned int) iy;

	if (ux>=uix) 
		mx = uix+(ux-uix)/2;
	else mx = ux+(uix-ux)/2;
	if (uy>=uiy) 
		my = uiy+(uy-uiy)/2;
	else my = uy+(uiy-uy)/2;

	if ( ((mx==ux) && (my==uy)) || \
		((mx==uix) && (my==uiy)))
		return 0;
		
	if (image[my*xsize+mx]>=100)
		r=1;

	return r;
}

static int intersecta(double x, double y, double ix, double iy, int i, int j, double pb, double pp, int n, double * f, double * s, int dim) {
	int k;
	double jx, jy; /* x e y de la interseccion entre el segmento de entrada y un segmento de s[] fp[] */
	double mx, Mx, my, My;
	double mox, Mox, moy, Moy;

	int m = 0;

	for(k=0;k<n;k++) {
		if (k==i) continue;
		if (k==j) continue;

		// igualar las rectas y verificar que el punto de interseccion está dentro del segmentito
		jx = (f[k*2+1] - pb) / (pp - f[k*2]);
		jy = pp * jx + pb;
		mx = x <= ix ? x : ix;
		Mx = x >= ix ? x : ix; 
		my = y <= iy ? y : iy;
		My = y >= iy ? y : iy;

		mox = s[k*dim+0] <= s[k*dim+2] ? s[k*dim+0] : s[k*dim+2];
		Mox = s[k*dim+0] >= s[k*dim+2] ? s[k*dim+0] : s[k*dim+2];
		moy = s[k*dim+1] <= s[k*dim+3] ? s[k*dim+1] : s[k*dim+3];
		Moy = s[k*dim+1] >= s[k*dim+3] ? s[k*dim+1] : s[k*dim+3];

		if ( (mx <= jx) && (jx <= Mx) && (my <= jy) && (jy <=My) && \
		   (mox <= jx) && (jx <= Mox) && (moy <= jy) && (jy <=Moy))
				m++;
	}
	
	return m;

}


void pendientes(double * s, double * f, int n, int dim)
{
	int i;
	double x1, x2, y1, y2;

	/*
	 * (x-x1) / (x2-x1)   =   (y-y1)/(y2-y1)
	 * y = ( (y2-y1) / (x2-x1) * x)  - ( (y2-y1) / (x2-x1) * x1 )  + y1 
	 */

  for(i=0;i<n;i++)
  {
	  x1 = s[i*dim+0];
	  y1 = s[i*dim+1];
	  x2 = s[i*dim+2];
	  y2 = s[i*dim+3];
	
    /* pendiente */
	  f[i*2] = (y2-y1) / (x2-x1) ;
	  /* desplazamiento */
	  f[i*2+1] = -1 * ( (y2-y1) / (x2-x1) * x1 )  + y1 ;
  }
}



void grosor(double * s, double * f, int n, int dim, int xsize, int ysize, int cota_superior, int cota_inferior, double *image)
{
	/* x,y punto medio del segmento */
	int cant=0, sum=0;
	int i, j, k;
	double mx, my, Mx, My, pmx, pmy;
	double x, y; // x e y del punto origen 
	double pb, pp; // desplazamiento y pendiente de la perpendicular a un punto
	double ix, iy; // x e y de la interseccion entre las dos rectas 
	double d; //distancia entre dos puntos



/* Creamos un archivo eps para anexar */
  FILE * eps;
	char *filename = "grosordelpelo.eps";

  /* open file */
  if( strcmp(filename,"-") == 0 ) eps = stdout;
  else eps = fopen(filename,"w");
  if( eps == NULL ) {
	fprintf(stderr, "Error: unable to open EPS output file.\n");
	exit(1);
               }


  /* write EPS header */
  fprintf(eps,"%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(eps,"%%%%BoundingBox: 0 0 %d %d\n",xsize,ysize);
  fprintf(eps,"%%%%Creator: LSD, Line Segment Detector\n");
  fprintf(eps,"%%%%Title: (%s)\n",filename);
  fprintf(eps,"%%%%EndComments\n");

/* Fin de Creamos un archivo eps para anexar */



  /* para saber si son medidas buenas o malas y varianza y desviacion estandar*/
  int mi = 0; // indice del vector medidas
  int mi_old = 0; // indice del vector medidas
  int cant_old=1, sum_old=0;
  int k_old=0; // para saber si se hicieron las 3 mediciones en un segmento
       char lineas_rojas[600];
       char linea_roja[200];

  for (i=0;i<200;i++) medidas[i]=0;
  	
  // Esto es para no medir en la medula y en segmentos muy chicos
	int seg_muy_chico = 30;

  /* FIN de cargamos toda la imagen en un arreglo */
	for(i=0;i<n;i++) {
		mx = s[i*dim+0] <= s[i*dim+2] ? s[i*dim+0] : s[i*dim+2];
		Mx = s[i*dim+0] >= s[i*dim+2] ? s[i*dim+0] : s[i*dim+2];
		my = s[i*dim+1] <= s[i*dim+3] ? s[i*dim+1] : s[i*dim+3];
		My = s[i*dim+1] >= s[i*dim+3] ? s[i*dim+1] : s[i*dim+3];

		/* Punto medio del segmento actual */
		pmx = mx+(Mx-mx)/2;
		pmy = my+(My-my)/2;

		/* Si el segmento es muy chico entonces lo descartamos como RUIDO */
		// Esto es para no medir en la medula y en segmentos muy chicos
		d = sqrt(  pow((Mx - mx),2) + pow((My-my ),2) );
		if (d < seg_muy_chico) continue;
		pb = s[i*dim+1] - (   ((-1) * 1/f[i*2] ) * s[i*dim+0]  );
		pp = (-1) * 1/f[i*2] ;
		

		/* para saber si son medidas buenas o malas */
        sum_old = sum;
        cant_old = cant;
        mi_old = mi;
        k_old = 0;
               lineas_rojas[0] = '\0';
               linea_roja[0] = '\0';


		for(k=0;k<3;k++) {
			/* Por cada segmento tomamos 3 puntos, inicio, medio y final */
			switch (k) {
			    case 0:
                    pb = s[i*dim+1] - (   ((-1) * 1/f[i*2] ) * s[i*dim+0]  );
				    x=s[i*dim+0];
				    y=s[i*dim+1];
				    break;
			    case 1:
                    pb = pmy - (   ((-1) * 1/f[i*2] ) * pmx  );
				    x=pmx;
				    y=pmy;
				    break;
			    case 2:
                    pb = s[i*dim+3] - (   ((-1) * 1/f[i*2] ) * s[i*dim+2]  );
				    x=s[i*dim+2];
				    y=s[i*dim+3];
				    break;
			}

		  for(j=0;j<n;j++) {
			  if (j==i) continue;

			  // igualar las rectas y verificar que el punto de interseccion está dentro del segmentito
			  // Tambien podriamos verificar que el x del inicio de la perpendicular esté a la izquierda
        // del punto que intersecta en el segmentito (la pistola deberia tomar las fotos con los pelos en vertical).
        // Entonces siempre analizamos "perpendiculares" que van a la derecha del punto de origen.
			  /* y = M*x + N
			   * y2 = M2*x + N2
   			   * N - N2 = (M2 - M) * x
   			   * x = (N - N2) / (M2 - M)
			   */
        ix = (f[j*2+1] - pb) / (pp - f[j*2]);
        iy = pp * ix + pb;
        mx = s[j*dim+0] <= s[j*dim+2] ? s[j*dim+0] : s[j*dim+2];
                Mx = s[j*dim+0] >= s[j*dim+2] ? s[j*dim+0] : s[j*dim+2];
                my = s[j*dim+1] <= s[j*dim+3] ? s[j*dim+1] : s[j*dim+3];
                My = s[j*dim+1] >= s[j*dim+3] ? s[j*dim+1] : s[j*dim+3];

        /* Si el segmento es muy chico entonces lo descartamos como RUIDO */
		    // distancia entre los dos puntos 
		    // d = sqrt(  (x2 - x1)^2 + (y2-y1)^2 )
		    // Esto es para no medir en la medula y en segmentos muy chicos
		    d = sqrt(  pow((Mx - mx),2) + pow((My-my ),2) );
		    if (d < seg_muy_chico) continue;
			  if ((mx >= ix) || (Mx <= ix) ) continue;
        if ((my >= iy) || (My <= iy) ) continue;
				
			  // Si los segmentos NO son paralelos entonces suponemos que NO son del mismo pelo
			  // (pendientes distintas). Si las pendientes son "bastante" cercanas (casi paralelas), aceptamos el segmento como valido
			  double xp;
			  double yp;
			  double b;
			  double c;
			  /* TODO : el 0.1 tiene que ser "definible", para indicar "cuan" paralela aceptamos los segmentos opuestos */

			  /* si x1 y x2 son iguales entonces el pelo es vertical, es un caso extremo para las pendientes */
			  //xp = s[i*dim+0] >= s[i*dim+2] ? s[i*dim+0] - s[i*dim+2]: s[i*dim+2] - s[i*dim+0];
			  if (s[i*dim+0] >= s[i*dim+2])
			    xp = s[i*dim+0] - s[i*dim+2];
			  else
			    xp = s[i*dim+2] - s[i*dim+0];

			  //b = s[j*dim+0] >= s[j*dim+2] ? s[j*dim+0] - s[j*dim+2]: s[j*dim+2] - s[j*dim+0];
			  if (s[j*dim+0] >= s[j*dim+2])
			    b = s[j*dim+0] - s[j*dim+2];
			  else
			    b = s[j*dim+2] - s[j*dim+0];

			  if ((xp <= 0.1) && (b > 0.1))
			    continue;

			  /* si y1 y y2 son iguales entonces el pelo es horizontal, es un caso extremo para las pendientes */
			  yp = s[i*dim+1] >= s[i*dim+3] ? s[i*dim+1] - s[i*dim+3]: s[i*dim+3] - s[i*dim+1];
			  b = s[j*dim+1] >= s[j*dim+3] ? s[j*dim+1] - s[j*dim+3]: s[j*dim+3] - s[j*dim+1];
			  if ((xp > 0.1) && (yp <= 0.1) && (b > 0.1))
			    continue;
			
			  /* si el pelo esta inclinado nos fijamos si son paralelos los segmentos analizados */
			  // a = s[i*dim+0] >= s[i*dim+2] ? s[i*dim+0] - s[i*dim+2]: s[i*dim+2] - s[i*dim+0];
			  // b = s[i*dim+1] >= s[i*dim+3] ? s[i*dim+1] - s[i*dim+3]: s[i*dim+3] - s[i*dim+1];
			  c= f[i*2] >= f[j*2] ? f[i*2] - f[j*2] : f[j*2] - f[i*2];
			  if ((xp > 0.1) && (yp > 0.1) && (c > 0.1)) continue;
			
			  /* Si la perpendicular intersecta algun otro segmento entonces 
			   * es una perpendicular confusa, con ruido 
			   */
			  if (intersecta(x, y, ix, iy, i, j, pb, pp, n, f, s, dim)) continue;

			  /* Verificamos si el punto medio de la perpendicular está dentro o fuera de un pelo */
			  //mx = x>=ix ? ix+(x-ix)/2 : x+(ix-x)/2;
			  //if ((x==ix) || (mx==0)) mx=x;
    
			  //my = y>=iy ? iy+(y-iy)/2 : y+(iy-y)/2;
			  //if ((y==iy) || (my==0)) my=y;
			  //printf("color=%i \n", pixels[(int)(my*640+mx)]);
			  //printf("%f %f %i\n", x, y, pixels[(unsigned int)(my*640+mx)]);
        //printf("%f %f %i\n", mx, my, pixels[(unsigned int)(my*480+mx)]);
			  // if (pixels[(unsigned int)((my)*480+mx)]>=200)
			  //if (pixels[(unsigned int)(y*480+x)]>=200)
			  if (es_fondo(x, y, ix, iy, image, xsize, ysize)) continue;


        // distancia entre los dos puntos 
        // d = sqrt(  (x2 - x1)^2 + (y2-y1)^2 )
        d = sqrt(  pow((ix - x),2) + pow((iy-y ),2) );
        /* TODO : el valor 100 o 70 tiene que ser "definible" para indicar cuales "largos" se descartan */
        if ((d < cota_superior) && (d > cota_inferior)) {
          /* Mostramos el segmento y distancia
    		   * printf("x1=%f, y1=%f, x2=%f, y2=%f, xj=%f, yj=%f . Distancia Pixels : %f\n", x, y, ix, iy, s[j*dim+0], s[j*dim+1], d);
			     */
			    sum = sum + d;
			    cant++;
    
          k_old++;
          medidas[mi] = d; mi++;

          fprintf(stderr, "d = %f\n", d);



/* Agregamos datos al archivo grosordelpelo.eps */
      // fprintf( eps,"newpath %f %f moveto %f %f lineto 1 0 0 setrgbcolor 4  setlinewidth stroke\n",
 sprintf(linea_roja, "newpath %f %f moveto %f %f lineto 1 0 0 setrgbcolor 4  setlinewidth stroke\n",
x,
(double) ysize - y,
               ix, 
(double) ysize - iy
                 );

       strcat(lineas_rojas, linea_roja);

/* Fin de Agregamos datos al archivo grosordelpelo.eps */


    		}
		  }		/* del for j */
		}		/* del for k */
		
		/* Si hay al menos 2 medidas de 3 buenas entonces las aceptamos como validas */
    if (k_old < 2) {
      sum = sum_old;
      cant = cant_old;
      mi = mi_old;
      fprintf(stderr, "Medidas malas \n");
    } else {
                       fprintf( eps,"%s", lineas_rojas);

      fprintf(stderr, "Medidas buenas \n");
    }
	}
	if (cant==0) cant=1;
	if (mi==0) mi=1;
	calcular_varianza_desvio_estandar(medidas, sum, cant, mi); // mi es el indice del vector, nro de medidas
	printf("Grosor del PELO en pixels : %f\n", (double)sum/cant);



/* Cerramos al archivo grosordelpelo.eps */
  fprintf(eps,"showpage\n");
  fprintf(eps,"%%%%EOF\n");
  if( eps != stdout && fclose(eps) == EOF ) {
	fprintf(stderr, "Error: unable to close file while writing EPS file.\n");
	exit(1);
               }
/* Fin de Cerramos al archivo grosordelpelo.eps */



}

/* Creamos el arreglo de pendientes y desplazamiento */
/* dos lugares por cada segmento : pendiente y desplazamiento */
//static double fcs[n][2];

void fibras(double * s, int n, int dim, int xsize, int ysize, int cota_superior, int cota_inferior, double *image)
{
  double * fcs;

  /* Creamos el arreglo de funciones */
  /* dos lugares por funcion : pendiente y desplazamiento */
  fcs = (double *)malloc(n*2*sizeof(double));
	
	pendientes(s, (double *)fcs, n, dim);
	grosor(s, (double *)fcs, n, dim, xsize, ysize, cota_superior, cota_inferior, image);
  
  free(fcs);
}

