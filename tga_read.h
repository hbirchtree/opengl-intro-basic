#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#pragma pack(1)

typedef struct tga_header_t
{
    char idlen;
    char maptype;
    char typecode;
    short int colmaporigin;
    short int colmaplen;
    char colmapdepth;
    short int origin_x;
    short int origin_y;
    short int width;
    short int height;
    char bpp;
    char imagedesc;
} tga_header;

typedef struct tga_data_t
{
    tga_header* header;
    void* file_data;
    void* img_data;
    size_t size;
} tga_data;

tga_data* get_texture_data(const char* texfile)
{
    FILE* rw = fopen(texfile,"rb");
    if(!rw)
        return 0x0;

    tga_data* tga = malloc(sizeof(tga_data));

	fseek(rw, 0, FILE_END);
    tga->size = ftell(rw);
	fseek(rw, 0, FILE_BEGIN);
    tga->file_data = malloc(tga->size);
	unsigned char* data = (unsigned char*)tga->file_data;
	size_t out = fread(tga->file_data,tga->size,1,rw);
    fclose(rw);

	fprintf(stderr,"Got bad read size: %i",out);

    tga->header = (tga_header*)tga->file_data;

	printf("Offsets: %p, %p\n", &tga->header->idlen, &tga->header->imagedesc);
	printf("Sizes: %i, %i\n",sizeof(char),sizeof(short int));

    tga->img_data = (char*)tga->file_data + sizeof(tga_header) + tga->header->idlen;

    return tga;
}

void free_texture_data(tga_data* data)
{
    free(data->file_data);
    free(data);
}
