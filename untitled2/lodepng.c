#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lodepng.h"

// принимаем на вход: имя файла, указатели на int для хранения прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную память для хранения картинки
// Если память выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height)
{
    unsigned char* image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error != 0) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    return image;
}

// принимаем на вход: имя файла для записи, указатель на массив пикселей,  ширину и высоту картинки
// Если преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
    unsigned char* png;
    size_t pngsize;
    int error = lodepng_encode32(&png, &pngsize, image, width, height);
    if (error == 0) {
        lodepng_save_file(png, pngsize, filename);
    } else {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(png);
}


// вариант огрубления серого цвета в ЧБ
void contrast(unsigned char *col, int bw_size)
{
    for (int i = 0; i < bw_size; i++) {
        if (col[i] < 55)
            col[i] = 0;
        if (col[i] > 195)
            col[i] = 255;
    }
}


// Гауссово размыттие
void Gauss_blur(unsigned char *col, unsigned char *blr_pic, int width, int height)
{
    int i, j;
    for(i=1; i < height-1; i++)
        for(j=1; j < width-1; j++)
        {
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)];
        }
    return;
}

//  Место для экспериментов
void color(unsigned char *blr_pic, unsigned char *res, int size)
{
    int i;
    for(i=1;i<size;i++)
    {
        res[i*4]=40+blr_pic[i]+0.35*blr_pic[i-1];
        res[i*4+1]=65+blr_pic[i];
        res[i*4+2]=170+blr_pic[i];
        res[i*4+3]=255;
    }
    return;
}



void remake_BW(unsigned char *foto, int width, int height, unsigned char *BW_foto)
{
    for (int i = 0; i < width * height; i++) {
        BW_foto[i] = (unsigned char)(
            foto[4 * i    ] * 0.299 +   /* R */
            foto[4 * i + 1] * 0.587 +   /* G */
            foto[4 * i + 2] * 0.114     /* B */
        );
    }
}


int bfs_component(unsigned char *bw_foto, int *visited, int width, int height, int start_x, int start_y, int threshold)
{

    int *queue = (int*)malloc(width * height * sizeof(int));
    if (!queue) return 0;

    int head = 0, tail = 0;
    int count = 0;

    int start_idx = start_y * width + start_x;
    queue[tail++] = start_idx;
    visited[start_idx] = 1;

    int dx[8] = { 1, -1,  0,  0,  1, -1,  1, -1 };
    int dy[8] = { 0,  0,  1, -1,  1,  1, -1, -1 };

    while (head < tail) {
        int idx = queue[head++];
        int cx = idx % width;
        int cy = idx / width;
        count++;

        for (int k = 0; k < 8; k++) {
            int nx = cx + dx[k];
            int ny = cy + dy[k];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int nidx = ny * width + nx;
                if (!visited[nidx] && bw_foto[nidx] >= threshold) {
                    visited[nidx] = 1;
                    queue[tail++] = nidx;
                }
            }
        }
    }

    free(queue);
    return count;
}


int tanker_counter(unsigned char *bw_foto, int width, int height, int threshold, int min_size, int max_size)
{
    int *visited = (int*)calloc(width * height, sizeof(int));
    int tanker_count = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            if (!visited[idx] && bw_foto[idx] >= threshold) {
                int size = bfs_component(bw_foto, visited, width, height, x, y, threshold);
                if (size >= min_size && size <= max_size) {
                    tanker_count++;
                }
            }
        }
    }

    free(visited);
    return tanker_count;
}

int main(int argc, char *argv[])
{
    const char* filename = "skull.png";
    unsigned int width, height;
    int size;

    // Прочитали картинку
    unsigned char* picture = load_png("skull.png", &width, &height);
    if (picture == NULL)
    {
        printf("Problem reading picture from the file %s. Error.\n", filename);
        return -1;
    }


    int bw_size = width * height;

    unsigned char *bw_pic  = (unsigned char*)malloc(bw_size);
    unsigned char *blr_pic = (unsigned char*)malloc(bw_size);
    unsigned char *finish  = (unsigned char*)malloc(bw_size * 4);


    remake_BW(picture, width, height, bw_pic);


    contrast(bw_pic, bw_size);

    color(bw_pic, finish, bw_size);
    write_png("contrast.png", finish, width, height);

    Gauss_blur(bw_pic, blr_pic, width, height);
    color(blr_pic, finish, bw_size);
    write_png("gauss.png", finish, width, height);


    color(blr_pic, finish, bw_size);
    write_png("picture_out.png", finish, width, height);

    int count = tanker_counter(bw_pic, (int)width, (int)height, /*threshold=*/255, /*min_size=*/3, /*max_size=*/5);
    printf("tankers count: %d\n", count);

    // не забыли почистить память!
    free(bw_pic);
    free(blr_pic);
    free(finish);
    free(picture);

    return 0;
}
