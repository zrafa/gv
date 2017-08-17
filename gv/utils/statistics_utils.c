#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void calcular_varianza_desvio_estandar(double * medidas, int sum, int cant, int mi) {
    double varianza = 0;
    double desv_estandar = 0;
    double media = (double) sum/cant;
    int cant_en_desviacion = 0;     // cantidad de medidas dentro de la desviacion estandar
    int i;

    for (i=0;i<mi;i++) 
        varianza = varianza + pow((medidas[i]-media) , 2);
    varianza = varianza / mi;
    desv_estandar = sqrt(varianza);
    for (i=0;i<mi;i++) 
        if (((media - desv_estandar) <= medidas[i])   &&  (medidas[i] <= (media + desv_estandar)))
            cant_en_desviacion++;

    fprintf(stderr, "\n\tNro de Mediciones = %i \tMedia = %f\n", mi, media);
    fprintf(stderr, "\tVarianza : %f\n", varianza);
    fprintf(stderr, "\tDesviacion estandar : %f\n", sqrt(varianza));
    fprintf(stderr, "\tMedidas dentro de la desviacion estandar : %i (%i%%)\n\n", cant_en_desviacion, cant_en_desviacion*100/mi);
}



