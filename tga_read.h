#include <stdlib.h>
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
    SDL_RWops* rw = SDL_RWFromFile(texfile,"r");
    if(!rw)
        return 0x0;

    tga_data* tga = new tga_data;

    tga->size = SDL_RWsize(rw);
    tga->file_data = malloc(tga->size);
    SDL_RWread(rw,tga->file_data,tga->size,tga->size);
    SDL_RWclose(rw);

    tga->header = (tga_header*)tga->file_data;

    tga->img_data = (char*)tga->file_data + sizeof(tga_header) + tga->header->idlen;

    return tga;
}

void free_texture_data(tga_data* data)
{
    free(data->file_data);
    free(data);
}
