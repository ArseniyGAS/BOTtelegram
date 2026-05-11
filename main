#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lodepng.h"

unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height)
{
    unsigned char* pixels = NULL;
    int error = lodepng_decode32_file(&pixels, width, height, filename);
    if (error != 0) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    return pixels;
}

void write_png(const char* filename, const unsigned char* pixels, unsigned width, unsigned height)
{
    unsigned char* png_buf;
    size_t png_size;
    int error = lodepng_encode32(&png_buf, &png_size, pixels, width, height);
    if (error == 0) {
        lodepng_save_file(png_buf, png_size, filename);
    } else {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(png_buf);
}

/* тёмные пиксели → чёрные, светлые → белые, средние не трогаем */
void contrast(unsigned char *gray, int num_pixels)
{
    for (int i = 0; i < num_pixels; i++) {
        if (gray[i] < 55)
            gray[i] = 0;
        if (gray[i] > 195)
            gray[i] = 255;
    }
}

void gauss_blur(unsigned char *src, unsigned char *dst, int width, int height)
{
    int row, col;
    for (row = 1; row < height - 1; row++)
        for (col = 1; col < width - 1; col++)
        {
            /* крестообразные соседи */
            dst[width*row+col] = 0.084*src[width*row+col]     + 0.084*src[width*(row+1)+col]
                                + 0.084*src[width*(row-1)+col] + 0.084*src[width*row+(col+1)]
                                + 0.084*src[width*row+(col-1)];
            /* диагональные соседи */
            dst[width*row+col] += 0.063*src[width*(row+1)+(col+1)] + 0.063*src[width*(row+1)+(col-1)]
                                +  0.063*src[width*(row-1)+(col+1)] + 0.063*src[width*(row-1)+(col-1)];
        }
}

void colorize(unsigned char *gray, unsigned char *rgba, int num_pixels)
{
    for (int i = 1; i < num_pixels; i++)
    {
        rgba[i*4]   = 40  + gray[i] + 0.35*gray[i-1];
        rgba[i*4+1] = 65  + gray[i];
        rgba[i*4+2] = 170 + gray[i];
        rgba[i*4+3] = 255;
    }
}

/* стандартные веса яркости — глаз лучше видит зелёный, хуже синий */
void rgba_to_gray(unsigned char *rgba, int width, int height, unsigned char *gray)
{
    for (int i = 0; i < width * height; i++) {
        gray[i] = (unsigned char)(
            rgba[4*i  ] * 0.299 +
            rgba[4*i+1] * 0.587 +
            rgba[4*i+2] * 0.114
        );
    }
}

int bfs_component(unsigned char *gray, int *visited, int width, int height,
                  int start_x, int start_y, int threshold)
{
    int *queue = (int*)malloc(width * height * sizeof(int));
    if (!queue) return 0;

    int head = 0, tail = 0, count = 0;

    int start_idx = start_y * width + start_x;
    queue[tail++] = start_idx;
    visited[start_idx] = 1;

    /* 8 направлений, включая диагонали */
    int dx[8] = {  1, -1,  0,  0,  1, -1,  1, -1 };
    int dy[8] = {  0,  0,  1, -1,  1,  1, -1, -1 };

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
                if (!visited[nidx] && gray[nidx] >= threshold) {
                    visited[nidx] = 1;
                    queue[tail++] = nidx;
                }
            }
        }
    }

    free(queue);
    return count;
}

int count_ships(unsigned char *gray, int width, int height,
                int threshold, int min_size, int max_size)
{
    int *visited = (int*)calloc(width * height, sizeof(int));
    int ship_count = 0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int idx = row * width + col;
            if (!visited[idx] && gray[idx] >= threshold) {
                int blob_size = bfs_component(gray, visited, width, height, col, row, threshold);
                if (blob_size >= min_size && blob_size <= max_size)
                    ship_count++;
            }
        }
    }

    free(visited);
    return ship_count;
}

int main(int argc, char *argv[])
{
    const char* filename = "skull.png";
    unsigned int width, height;

    unsigned char* picture = load_png(filename, &width, &height);
    if (picture == NULL) {
        printf("не удалось открыть %s\n", filename);
        return -1;
    }

    int num_pixels = width * height;

    unsigned char *gray_pic  = (unsigned char*)malloc(num_pixels);
    unsigned char *blur_pic  = (unsigned char*)malloc(num_pixels);
    unsigned char *rgba_out  = (unsigned char*)malloc(num_pixels * 4);

    rgba_to_gray(picture, width, height, gray_pic);

    contrast(gray_pic, num_pixels);

    colorize(gray_pic, rgba_out, num_pixels);
    write_png("contrast.png", rgba_out, width, height);

    gauss_blur(gray_pic, blur_pic, width, height);
    colorize(blur_pic, rgba_out, num_pixels);
    write_png("gauss.png", rgba_out, width, height);

    /* threshold=255 — только белые пятна после контраста,
       min=3..max=5 подобрать под конкретный снимок */
    int ships = count_ships(gray_pic, (int)width, (int)height, 255, 3, 5);
    printf("ships found: %d\n", ships);

    free(gray_pic);
    free(blur_pic);
    free(rgba_out);
    free(picture);

    return 0;
}
