#include <stdio.h>
#include <stdlib.h>

void rawWrite(unsigned char* buffer_start, int length, char* rawFilename)
{
    FILE * f;

    /* open file */
    f = fopen(rawFilename,"wb");
    if( f == NULL ) {
        fprintf(stderr, "Error: unable to open output image file.");
        exit(EXIT_FAILURE);
    }

    /* write data */
    fwrite(buffer_start, 1, length, f);

    /* close file */
    fclose(f);
}


