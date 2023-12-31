
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "stb_image.h" //PNG Decoder

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" //PNG Encoder

#define QOI_IMPLEMENTATION
#include "qoi.h"

#define STR_ENDS_WITH(S, E) (strcmp(S + strlen(S) - (sizeof(E) - 1), E) == 0)

// Function to concatenate three strings
char *concatenateThreeStrings(const char *str1, const char *str2, const char *str3)
{
    // Calculate the total length of the concatenated string
    size_t totalLength = strlen(str1) + strlen(str2) + strlen(str3);

    // Allocate memory for the concatenated string
    char *result = (char *)malloc(totalLength + 1);

    if (result == NULL)
    {
        // Memory allocation failed
        return NULL;
    }

    // Copy the first string into the result
    strcpy(result, str1);

    // Concatenate the second string to the result
    strcat(result, str2);

    // Concatenate the third string to the result
    strcat(result, str3);

    return result;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        puts("Usage: qoibench <infile> <bench_name>");
        puts("Examples:");
        puts("  qoiconv input.png pic-01");
        exit(1);
    }

    void *pixels = NULL;
    int w, h, channels;
    if (STR_ENDS_WITH(argv[1], ".png"))
    {
        if (!stbi_info(argv[1], &w, &h, &channels))
        {
            printf("Couldn't read header %s\n", argv[1]);
            exit(1);
        }

        // Force all odd encodings to be RGBA
        if (channels != 3)
        {
            channels = 4;
        }

        pixels = (void *)stbi_load(argv[1], &w, &h, NULL, channels);
    }
    else if (STR_ENDS_WITH(argv[1], ".qoi"))
    {
        qoi_desc desc;
        pixels = qoi_read(argv[1], &desc, 0);
        channels = desc.channels;
        w = desc.width;
        h = desc.height;
    }

    if (pixels == NULL)
    {
        printf("Couldn't load/decode %s\n", argv[1]);
        exit(1);
    }

    // We found
    // Width , Hight , Channels , Raw Pixels

    clock_t start, end;
    double cpu_time_used;

//--------------------------------------------------------------------------------------------------------------
    // First we have to store binary data in SB


    int decodedLength = w * h * channels;
    char *name = concatenateThreeStrings("SB/", argv[2], ".bin");
    if (name == NULL)
    {
        printf("Memory allocation of name failed.\n");
    }

    FILE *fp = fopen(name, "wb");
    fwrite(pixels, 1, decodedLength, fp);
    fclose(fp);

    printf("Wrote decoded Binary image (%i bytes)\n", decodedLength);

//--------------------------------------------------------------------------------------------------------------
    // Second we have to store png data in SP



    int png_encoded_size = 0;
    name = concatenateThreeStrings("SP/", argv[2], ".png");
    if (name == NULL)
    {
        printf("Memory allocation of name failed.\n");
    }
    
    start = clock();
    png_encoded_size = stbi_write_png(name, w, h, channels, pixels, 0);
    end = clock();


    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;


    if (!png_encoded_size) {
    	printf("Couldn't write/encode png %s\n", argv[2]);
    	exit(1);
    } else {
        printf("\nPNG ENCODE TIME -\t%lf \tSIZE -\t%d\n", cpu_time_used, png_encoded_size);
    }


   //--------------------------------------------------------------------------------------------------------------
    // Third we have to store png data in SQ
  
    int qoi_encoded_size = 0;
    name = concatenateThreeStrings("SQ/", argv[2], ".qoi");
    if (name == NULL)
    {
        printf("Memory allocation of name failed.\n");
    }

    start = clock();
    qoi_encoded_size = qoi_write(name, pixels, &(qoi_desc){
    	.width = w,
    	.height = h,
    	.channels = channels,
    	.colorspace = QOI_SRGB
    });
    end = clock();

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    if (!qoi_encoded_size) {
    	printf("Couldn't write/encode qoi %s\n", argv[2]);
    	exit(1);
    } else {
        printf("\nQOI ENCODE TIME -\t%lf \tSIZE - \t%d\n", cpu_time_used, qoi_encoded_size);
    }

    free(pixels);
    free(name);

    //DECODE TIME CALCULATION

    //PNG Decode Time
    name = concatenateThreeStrings("SP/", argv[2], ".png");
    if (name == NULL)
    {
        printf("Memory allocation of name failed.\n");
    }
    if (!stbi_info(name, &w, &h, &channels))
    {
        printf("Couldn't read header %s\n", argv[1]);
        exit(1);
    }

    // Force all odd encodings to be RGBA
    if (channels != 3)
    {
        channels = 4;
    }

    start = clock();
    pixels = (void *)stbi_load(argv[1], &w, &h, NULL, channels);
    end = clock();
    if (pixels == NULL)
    {
        printf("Couldn't load/decode %s\n", name);
        exit(1);
    }else{
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("\nPNG DECODE TIME -\t%lf\n", cpu_time_used);
    }


    //QOI Decode Time
    name = concatenateThreeStrings("SQ/", argv[2], ".qoi");
    if (name == NULL)
    {
        printf("Memory allocation of name failed.\n");
    }

        qoi_desc desc;

        start = clock();
        pixels = qoi_read(name , &desc, 0);
        end = clock();

        if (pixels == NULL)
        {
            printf("Couldn't load/decode %s\n", name);
            exit(1);
        }else{
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("\nQOI DECODE TIME -\t%lf\n", cpu_time_used);
        }




    return 0;
}
