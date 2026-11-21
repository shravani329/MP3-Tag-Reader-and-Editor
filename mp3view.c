#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mp3view.h"

/* Copy frame content into destination variable */
static void copy_frame_text(char *dest, int capacity,unsigned char *data,
                                unsigned int size, int is_comment)
{
    unsigned int start = 1;

    /* Support the common ISO-8859-1 text encoding */
    if (size == 0 || data[0] != 0)
        return;

    if (is_comment)
    {
        /* Skip encoding and 3-byte language code */
        start = 4;

        /* Skip comment description */
        while (start < size && data[start] != '\0')
            start++;

        if (start < size)
            start++;
    }

    int i = 0;

    while (start < size && data[start] != '\0' && i < capacity - 1)
    {
        dest[i++] = data[start++];
    }

    dest[i] = '\0';
}

void view_operation(FILE *fp)
{
    char title[100] = "";
    char artist[100] = "";
    char album[100] = "";
    char year[20] = "";
    char music[100] = "";
    char comment[200] = "";

    unsigned char frame_id[5];
    unsigned char size_bytes[4];
    unsigned char flags[2];

    /* Skip ID3 header (10 bytes) */
    fseek(fp, 10, SEEK_SET);

    while (1)
    {
        /* Read frame ID */
        if (fread(frame_id, 1, 4, fp) != 4)
            break;

        frame_id[4] = '\0';

        /* Stop when padding or invalid frame ID is encountered */
        int valid = 1;

        for (int i = 0; i < 4; i++)
        {
            if (!((frame_id[i] >= 'A' && frame_id[i] <= 'Z') ||
                  (frame_id[i] >= '0' && frame_id[i] <= '9')))
            {
                valid = 0;
                break;
            }
        }

        if (!valid)
            break;

        /* Read frame size */
        if (fread(size_bytes, 1, 4, fp) != 4)
            break;

        /* Read frame flags */
        if (fread(flags, 1, 2, fp) != 2)
            break;

        /* Convert 4-byte big-endian size into integer */
        unsigned int frame_size =
            ((unsigned int)size_bytes[0] << 24) |
            ((unsigned int)size_bytes[1] << 16) |
            ((unsigned int)size_bytes[2] << 8) |
            (unsigned int)size_bytes[3];

        if (frame_size > 1024 * 1024)
            break;

        /* Allocate memory for frame content */
        unsigned char *data = malloc(frame_size + 1);

        if (data == NULL)
            break;

        /* Read frame content */
        if (fread(data, 1, frame_size, fp) != frame_size)
        {
            free(data);
            break;
        }

        data[frame_size] = '\0';

        /* Check frame ID and copy its content */

        if (strcmp((char *)frame_id, "TIT2") == 0)
        {
            copy_frame_text(title, sizeof(title),
                            data, frame_size, 0);
        }
        else if (strcmp((char *)frame_id, "TPE1") == 0)
        {
            copy_frame_text(artist, sizeof(artist),
                            data, frame_size, 0);
        }
        else if (strcmp((char *)frame_id, "TALB") == 0)
        {
            copy_frame_text(album, sizeof(album),
                            data, frame_size, 0);
        }
        else if (strcmp((char *)frame_id, "TYER") == 0)
        {
            copy_frame_text(year, sizeof(year),
                            data, frame_size, 0);
        }
        else if (strcmp((char *)frame_id, "TCON") == 0)
        {
            copy_frame_text(music, sizeof(music),
                            data, frame_size, 0);
        }
        else if (strcmp((char *)frame_id, "COMM") == 0)
        {
            copy_frame_text(comment, sizeof(comment),
                            data, frame_size, 1);
        }

        free(data);
    }

    /* Display extracted information */

    printf("\n--------------- SELECTED VIEW DETAILS ---------------\n");

    printf("\n--------------- MP3 TAG READER AND EDITOR FOR ID3v2 ---------------\n");

    printf("\nTITLE   : %s", title);
    printf("\nARTIST  : %s", artist);
    printf("\nALBUM   : %s", album);
    printf("\nYEAR    : %s", year);
    printf("\nMUSIC   : %s", music);
    printf("\nCOMMENT : %s", comment);

    printf("\n\n--------------- DETAILS DISPLAYED SUCCESSFULLY ---------------\n");
}
