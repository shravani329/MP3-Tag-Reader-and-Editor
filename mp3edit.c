#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mp3edit.h"

 /* Find the frame ID based on the edit option */
static char *get_frame_id(char *option)
{
    if (strcmp(option, "-t") == 0)
        return "TIT2";

    else if (strcmp(option, "-a") == 0)
        return "TPE1";

    else if (strcmp(option, "-A") == 0)
        return "TALB";

    else if (strcmp(option, "-y") == 0)
        return "TYER";

    else if (strcmp(option, "-m") == 0)
        return "TCON";

    else if (strcmp(option, "-c") == 0)
        return "COMM";

    return NULL;
}

/* Convert 4-byte big-endian size into integer */
static unsigned int get_frame_size(unsigned char *size_bytes)
{
    return ((unsigned int)size_bytes[0] << 24) |
           ((unsigned int)size_bytes[1] << 16) |
           ((unsigned int)size_bytes[2] << 8) |
           (unsigned int)size_bytes[3];
}

/* Convert integer into 4-byte big-endian size */
static void set_frame_size(unsigned char *size_bytes,
                           unsigned int size)
{
    size_bytes[0] = (size >> 24) & 0xFF;
    size_bytes[1] = (size >> 16) & 0xFF;
    size_bytes[2] = (size >> 8) & 0xFF;
    size_bytes[3] = size & 0xFF;
}

 /* Update ID3 header tag size using synchsafe format */
static void set_tag_size(unsigned char *header, unsigned int size)
{
    header[6] = (size >> 21) & 0x7F;
    header[7] = (size >> 14) & 0x7F;
    header[8] = (size >> 7) & 0x7F;
    header[9] = size & 0x7F;
}

void edit_operation(FILE *fp, char *option, char *new_value)
{
    unsigned char header[10];
    unsigned char frame_header[10];

    char *target_frame = get_frame_id(option);

    if (target_frame == NULL)
    {
        printf("Invalid edit option\n");
        return;
    }

    /* Read ID3 header */
    rewind(fp);

    if (fread(header, 1, 10, fp) != 10)
    {
        printf("Unable to read ID3 header\n");
        return;
    }

    /* Check ID3 identifier */
    if (memcmp(header, "ID3", 3) != 0)
    {
        printf("Invalid MP3 file\n");
        return;
    }

    /* Check ID3 version 2.3 */
    if (header[3] != 3 || header[4] != 0)
    {
        printf("Only ID3v2.3 is supported\n");
        return;
    }

    /* Read total ID3 tag size */
    unsigned int tag_size =
        ((unsigned int)header[6] << 21) |
        ((unsigned int)header[7] << 14) |
        ((unsigned int)header[8] << 7) |
        (unsigned int)header[9];

    FILE *temp = tmpfile();

    if (temp == NULL)
    {
        printf("Unable to create temporary file\n");
        return;
    }

    unsigned int remaining = tag_size;
    unsigned int new_tag_size = tag_size;
    int found = 0;

    /* Write header temporarily */
    fwrite(header, 1, 10, temp);

    /* Read frames */
    while (remaining >= 10)
    {
        if (fread(frame_header, 1, 10, fp) != 10)
            break;

        remaining -= 10;

        /* Check for valid frame ID */
        int valid = 1;

        for (int i = 0; i < 4; i++)
        {
            if (!((frame_header[i] >= 'A' &&
                   frame_header[i] <= 'Z') ||
                  (frame_header[i] >= '0' &&
                   frame_header[i] <= '9')))
            {
                valid = 0;
                break;
            }
        }

        /* Preserve padding or remaining data */
        if (!valid)
        {
            fwrite(frame_header, 1, 10, temp);

            unsigned char buffer[4096];

            while (remaining > 0)
            {
                unsigned int count =
                    remaining > sizeof(buffer) ?
                    sizeof(buffer) : remaining;

                size_t bytes = fread(buffer, 1, count, fp);

                if (bytes == 0)
                    break;

                fwrite(buffer, 1, bytes, temp);
                remaining -= bytes;
            }

            break;
        }

        unsigned int old_size = get_frame_size(&frame_header[4]);

        if (old_size > remaining)
            break;

        unsigned char *data = malloc(old_size);

        if (data == NULL && old_size != 0)
            break;

        if (old_size > 0 &&
            fread(data, 1, old_size, fp) != old_size)
        {
            free(data);
            break;
        }

        remaining -= old_size;

        /* Check whether this is the requested frame */
        if (memcmp(frame_header, target_frame, 4) == 0)
        {
            unsigned char *new_data;
            unsigned int new_size;

            /* Normal text frames */
            if (strcmp(target_frame, "COMM") != 0)
            {
                new_size = strlen(new_value) + 1;

                new_data = malloc(new_size);

                if (new_data == NULL)
                {
                    free(data);
                    break;
                }

                new_data[0] = 0;
                memcpy(new_data + 1, new_value, strlen(new_value));
            }
            else
            {
                /* Preserve comment encoding, language and description */
                unsigned int prefix = 4;

                if (old_size < 4 || data[0] != 0)
                {
                    free(data);
                    printf("Unsupported comment frame format\n");
                    fclose(temp);
                    return;
                }

                while (prefix < old_size && data[prefix] != '\0')
                    prefix++;

                if (prefix >= old_size)
                {
                    free(data);
                    printf("Invalid comment frame\n");
                    fclose(temp);
                    return;
                }

                prefix++;

                new_size = prefix + strlen(new_value);

                new_data = malloc(new_size);

                if (new_data == NULL)
                {
                    free(data);
                    break;
                }

                memcpy(new_data, data, prefix);
                memcpy(new_data + prefix, new_value, strlen(new_value));
            }

            /* Update frame size */
            set_frame_size(&frame_header[4], new_size);

            /* Write modified frame */
            fwrite(frame_header, 1, 10, temp);
            fwrite(new_data, 1, new_size, temp);

            new_tag_size = new_tag_size - old_size + new_size;

            free(new_data);
            found = 1;
        }
        else
        {
            /* Copy unchanged frame */
            fwrite(frame_header, 1, 10, temp);

            if (old_size > 0)
                fwrite(data, 1, old_size, temp);
        }

        free(data);
    }

    if (!found)
    {
        printf("Requested tag not found\n");
        fclose(temp);
        return;
    }

    /* Update ID3 header size */
    set_tag_size(header, new_tag_size);

    fseek(temp, 0, SEEK_SET);
    fwrite(header, 1, 10, temp);

    /* Move to end of temporary file */
    fseek(temp, 0, SEEK_END);

    /* Copy remaining MP3 audio data */
    unsigned char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        fwrite(buffer, 1, bytes, temp);
    }

    /* Flush temporary file */
    fflush(temp);

    /* Copy edited data back into original file */
    rewind(temp);

    if (ftruncate(fileno(fp), 0) != 0)
    {
        printf("Unable to update original file\n");
        fclose(temp);
        return;
    }

    rewind(fp);

    while ((bytes = fread(buffer, 1, sizeof(buffer), temp)) > 0)
    {
        fwrite(buffer, 1, bytes, fp);
    }

    fflush(fp);

    fclose(temp);

    printf("Tag edited successfully!\n");
}

