#ifndef DECODE_H
#define DECODE_H
#define MAGIC_STRING_LENGTH 2
#include "types.h"

#define MAX_OUTPUT_BUF_SIZE 1
#define MAX_ENC_IMAGE_BUF_SIZE (MAX_OUTPUT_BUF_SIZE * 8)
#define MAX_OUTPUT_FILE_EXT 5

// Strucutre definition to store decoding data
typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    
    /* Decoded Magic string */
    char decoded_magic_string[MAGIC_STRING_LENGTH + 1];

    /* Output file extension details */
    uint size_output_fextn;
    char output_fextn[MAX_OUTPUT_FILE_EXT];

    /* Output file info */
    char *output_fname;
    FILE *fptr_output;
    uint size_secret_data;
} DecodeInfo;

/* Decoding function prototypes */

/* Read and validate Deccode args from argv */
Status read_and_validate_decode_args(char *argc[], DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status Open_files_for_decoding(DecodeInfo *decInfo, char *argv[]);

/* Decode Magic String*/
Status decode_magic_string(uint size, DecodeInfo *decInfo);

/* Decode output file extension */
Status decode_output_fextn(DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Decodes string from the image */
Status decode_data_from_image(uint size, char *data, FILE *fptr_src_image);

/* Store the decoded data in output file */
Status decode_data_to_output_file(DecodeInfo *decInfo);

/* Decode bytes from lsb of source image */
Status decode_byte_from_lsb(char *data, char *encoded_data);

/* Decode the size from source image */
uint get_size_from_image(FILE *fptr_src_image);


#endif