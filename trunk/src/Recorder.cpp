#include "Recorder.h"

#define PNG_STDIO_SUPPORTED
#include <png.h>

#pragma comment(lib, "libpng")

constexpr int bytes_per_pixel = 4;

//

void Recorder::SaveNewFrame(unsigned int *data) {
	char fn[250];
	sprintf_s(fn, "%s%d.png", path.c_str(), frame_id);
	SavePNG(fn, data);
	frame_id++;
}

void Recorder::SavePNG(const char *filename, unsigned int *data) {
	png_bytep *png_data = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for (int i = 0; i < height; ++i) {
		png_data[i] = (unsigned char*)data + i*width*bytes_per_pixel;
	}

	FILE *fp;
	fopen_s(&fp, filename, "wb");
	if(!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(png, info, width, height, 8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	//png_set_filler(png, 0, PNG_FILLER_AFTER);

	png_bytep *row_pointers = png_data;

	if (!row_pointers) abort();

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	fclose(fp);

	png_destroy_write_struct(&png, &info);

	free(png_data);
}
