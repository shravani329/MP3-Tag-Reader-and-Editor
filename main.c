#include <stdio.h>
#include <string.h>
#include "mp3view.h"
#include "mp3edit.h"

void display_help(void)
{
    printf("\n--------------- Help Menu ---------------\n");
    printf("-v sample.mp3             : View MP3 details\n");
    printf("-e -t value sample.mp3    : Edit Title\n");
    printf("-e -a value sample.mp3    : Edit Artist\n");
    printf("-e -A value sample.mp3    : Edit Album\n");
    printf("-e -y value sample.mp3    : Edit Year\n");
    printf("-e -m value sample.mp3    : Edit Genre\n");
    printf("-e -c value sample.mp3    : Edit Comments\n");
    printf("-h              : Help\n");
}

int main(int argc, char *argv[])
{
    FILE *fp;
    char *filename;

    // Check command-line arguments
    if (argc < 2)
    {
        printf("ERROR: Insufficient arguments\n");
        printf("Use -h for help\n");
        return 1;
    }

    //Help Validation
    if (strcmp(argv[1], "-h") == 0)
    {
        if (argc != 2)
        {
            printf("ERROR: Invalid help arguments\n");
            return 1;
        }

        display_help();
        return 0;
    }

    //View Validation
    if (strcmp(argv[1], "-v") == 0)
    {
        if (argc != 3)
        {
            printf("Usage: ./a.out -v sample.mp3\n");
            return 1;
        }

        filename = argv[2];

        //Validate file extension
        if (strlen(filename) < 4 ||
            strcmp(filename + strlen(filename) - 4, ".mp3") != 0)
        {
            printf("ERROR: Input file must have .mp3 extension\n");
            return 1;
        }

        // Open MP3 file
        fp = fopen(filename, "rb");

        if (fp == NULL)
        {
            printf("ERROR: Unable to open file\n");
            return 1;
        }

        printf("File opened Successfully!!\n");
        view_operation(fp);
        fclose(fp);
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        if (argc != 5)
        {
            printf("Usage: ./program -e -t NewTitle sample.mp3\n");
            return 1;
        }

        filename = argv[4];

        // Validate file extension
        if (strlen(filename) < 4 ||
            strcmp(filename + strlen(filename) - 4, ".mp3") != 0)
        {
            printf("ERROR: Input file must have .mp3 extension\n");
            return 1;
        }

        // Validate edit option
        if (strcmp(argv[2], "-t") != 0 &&
            strcmp(argv[2], "-a") != 0 &&
            strcmp(argv[2], "-A") != 0 &&
            strcmp(argv[2], "-y") != 0 &&
            strcmp(argv[2], "-m") != 0 &&
            strcmp(argv[2], "-c") != 0)
        {
            printf("ERROR: Invalid edit option\n");
            printf("Use -h for help\n");
            return 1;
        }
         // Open MP3 file in read and write binary mode
        fp = fopen(filename, "r+b");

        if (fp == NULL)
        {
            printf("Unable to open file");
            return 1;
        }

       edit_operation(fp, argv[2], argv[3]);
    }
    else
    {
        printf("ERROR: Invalid option\n");
        printf("Use -h for help\n");
        return 1;
    }

    return 0;
}
