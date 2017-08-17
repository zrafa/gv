#include <stdio.h>
#include <unistd.h>
#include <jpeglib.h>

long unsigned int RGB888toJPEG(unsigned char* img, int width, int height, int jpegQuality, unsigned char* buf)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	long unsigned int size;

	// create jpeg data
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &buf, &size);

	// set image parameters
	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	// set jpeg compression parameters to default
	jpeg_set_defaults(&cinfo);
	// and then adjust quality setting
	jpeg_set_quality(&cinfo, jpegQuality, TRUE);

	// start compress 
	jpeg_start_compress(&cinfo, TRUE);

	// feed data
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &img[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// finish compression
	jpeg_finish_compress(&cinfo);

	// destroy jpeg data
	jpeg_destroy_compress(&cinfo);

	return size;
}

