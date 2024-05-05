#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Read and validate Deccode args from argv 
 * Input: Command line arguments and decoding data 
 * Output: i/p file name 
 * Description: Read and validates the input arguments for decoding
 * Return value: d_success, d_failure
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check if the source image  is provided
    if (argv[2] == NULL)
    {
        printf("ERROR: Source image is not provided\n");
        return d_failure;
    }

    
    char *str;
    
    // Do error handling for source image
    if ((str = strstr(argv[2], ".bmp")) == NULL)
    {
        printf("ERROR: Unsupported format of Source image\n");
        printf("Usage: ./a.out -d <.bmp file> [output file]\n");
        return d_failure;
    }

    // Source image name
    decInfo->src_image_fname = argv[2];
    
    printf("INFO: ## Decoding Procedure Started ##\n");

    // do error handling for file openings
    if (Open_files_for_decoding(decInfo, argv) == e_failure)
    {
        printf("ERROR: Open_files_for_decoding function is failed\n");
        return 1;
    }
    else
    {
        printf("INFO: Done. Opened all required files\n");
    }
    
    return d_success;
}

/* Get File pointers for i/p and o/p files 
 * Input: Decoding data and command line arguments 
 * Output: File pointers to i/p and o/p files
 * Description: Gets the file pointers for i/p and o/p files
 * Return value: d_success, d_failure
 */
Status Open_files_for_decoding(DecodeInfo *decInfo, char *argv[])
{
    printf("INFO: Opening required files\n");
    /* Open source image */
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "r");

    // Do error handling 
    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
    	printf("ERROR: Unable to open file %s\n", decInfo->src_image_fname);
        return d_failure;
    }
    else
    {
        printf("INFO: Opened %s\n", decInfo->src_image_fname);
    }

    // Decoding magic string
    fseek(decInfo->fptr_src_image, 54, SEEK_SET);
    // Do error handling for magic string
    if (decode_magic_string(strlen(MAGIC_STRING), decInfo) == d_failure || strcmp(decInfo->decoded_magic_string, MAGIC_STRING))
    {
        printf("ERROR: This is not an encrypted file\n");
        return d_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }

    /* Decode output file extension */
    /* Do error hanlding for Output File Extension */
    if(decode_output_fextn(decInfo) == d_failure)
    {
        return d_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }

    // Open Output file
    if (argv[3] == NULL)
    {
        if(!strcmp(decInfo->output_fextn, ".txt"))
        {
            decInfo->output_fname = "decoded.txt";
        }
        else if(!strcmp(decInfo->output_fextn, ".c"))
        {
            decInfo->output_fname = "decoded.c";
        }
        else if(!strcmp(decInfo->output_fextn, ".sh"))
        {
            decInfo->output_fname = "decoded.sh";
        }
        else
        {
            puts("ERROR: Invalid format for decoded file\n");
            return d_failure;
        }
        printf("INFO: Output file not mentioned. Creating %s as default\n", decInfo->output_fname);
    }
    else
    {
        char *str;
        if((str = strchr(argv[3], '.')) == NULL)
        {
            strcat(argv[3], decInfo->output_fextn);
            decInfo->output_fname = argv[3];
        }
        else
        {
            strcpy(str, decInfo->output_fextn);
            decInfo->output_fname = argv[3];
        }
    }

    // Open output file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    // Do error handling for output file
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
    	printf("ERROR: Unable to open file %s\n", decInfo->output_fname);
        return d_failure;
    }
    else
    {
        printf("INFO: Opened %s\n", decInfo->output_fname);
    }

    // All files opened successfully
    return d_success;
}

/* Perform Decoding
 * Input: Decoding data
 * Output: Decoded Output file
 * Description: Decode the data from source image and stores in output file
 * Return value: d_failure, d_success
 */
Status do_decoding(DecodeInfo *decInfo)
{
    // Get the size of data
    printf("INFO: Decoding %s File Size\n", decInfo->output_fname);
    if((decInfo->size_secret_data = get_size_from_image(decInfo->fptr_src_image)) == 0)
    {
        printf("INFO: No Encoded data found\n");
        return d_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }
    // printf("Size of secret data: %u\n", decInfo->size_secret_data);

    // Decode and Store the secret data in output file
    if(decode_data_to_output_file(decInfo) != d_success)
    {
        printf("ERROR: do_decoding function failed\n");
        return d_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }

    // Close all opened files
    fclose(decInfo->fptr_src_image);
    fclose(decInfo->fptr_output);
    
    // Decoding done
    return d_success;

}

/* Decode Maigc String
 * Input: Magic string length and decoding data
 * Output: Decoded magic string from source image
 * Description: Decodes the magic string from source image
 * Return value: d_success, d_failure
 */
Status decode_magic_string(uint size, DecodeInfo *decInfo)
{
    printf("INFO: Decodeing Magic String Siganture\n");
    if(decode_data_from_image(size, decInfo->decoded_magic_string, decInfo->fptr_src_image) == d_success)
    {
        return d_success;
    }
    else
    {
        return d_failure;
    }
}

/* Decode output file extension
 * Input: Decoding data
 * Output: Output file extension
 * Description: Decodes the output extension from source image
 * Return value: d_failure, d_success
 */
Status decode_output_fextn(DecodeInfo *decInfo)
{
    printf("INFO: Decoding output file extension\n");
    // Get the size of extension
    if((decInfo->size_output_fextn = get_size_from_image(decInfo->fptr_src_image)) == 0)
    {
        printf("ERROR: failed to get the size of extension\n");
        return d_failure;
    }
    // printf("size of ouput extension: %d\n", decInfo->size_output_fextn);

    // Decode extension
    if(decode_data_from_image(decInfo->size_output_fextn, decInfo->output_fextn, decInfo->fptr_src_image) == d_success)
    {
        if(!(strcmp(decInfo->output_fextn, ".txt") && strcmp(decInfo->output_fextn, ".c") && strcmp(decInfo->output_fextn, ".sh")))
        {
            // printf("Encoded Extension is: %s\n", decInfo->output_fextn);
            return d_success;
        }
        else
        {
            printf("%s\n", decInfo->output_fextn);
            printf("ERROR: Unsupported format of secret file\n");
            return d_failure;
        }
    }
    else
    {
        printf("INFO: decode_data_from_image function failed\n");
        return d_failure;
    }
    
}

/* Decode data to ouptut fiel
 * Input: Decoding data
 * Output: Decoded output file
 * Description: Decodes the data from source image to output file
 * Return value: e_success
 */
Status decode_data_to_output_file(DecodeInfo *decInfo)
{    
    printf("INFO: Decoding %s File Data\n", decInfo->output_fname);
    // variable to store the decoded byte
    char decoded_data = 0;

    // Array to get encoded data from source image
    char encoded_data[MAX_ENC_IMAGE_BUF_SIZE] = {0};

    // Decode one byte from 8 bytes of encoded data
    for (int i = 0; i < decInfo->size_secret_data; i++)
    {
        decoded_data = 0;
        fread(encoded_data, sizeof(char), MAX_ENC_IMAGE_BUF_SIZE, decInfo->fptr_src_image);
        decode_byte_from_lsb(&decoded_data, encoded_data);
        fwrite(&decoded_data, sizeof(char), MAX_OUTPUT_BUF_SIZE, decInfo->fptr_output);
    }
    
    return d_success;
}

/* Decode byte from lsb of encoded source image
 * Input: one byte to store the decoded data and encoded_data 
 * Output: Decoded byte
 * Description: Decodes one byte from 8 bytes of encoded data
 * Return value: d_success
 */
Status decode_byte_from_lsb(char *byte, char *encoded_data)
{
   for (int i = 7, index = 0; index < 8; i--, index++)
   {
        *byte = *byte | (encoded_data[index] & 1) << i;
   }

   return d_success;
}

/* Get the size from image
 * Input: Source image pointer
 * Output: Decoded size
 * Description: Decodes the size from source image
 * return value: decoded size
 */
uint get_size_from_image(FILE *fptr_src_image)
{
    // get the encoded data from source image
    char encoded_size[MAX_ENC_IMAGE_BUF_SIZE * 4];

    // Variable to store decode size value
    uint decoded_size = 0;

    // Read 32 bytes from encoded image to decode 4 bytes of data (i.e. size)
    fread(encoded_size, sizeof(char), MAX_ENC_IMAGE_BUF_SIZE * 4, fptr_src_image);
    for (int i = 31, index = 0; index < MAX_ENC_IMAGE_BUF_SIZE * 4; i--, index++)
    {
        decoded_size = decoded_size | (encoded_size[index] & 1) << i;
    }
    return decoded_size;
}

/* Decodes String from source image
 * Input: Size of string, array to store the decoded string and source file pointer
 * Output: Decoded string
 * Description: Decodes a string from source image
 * Return value: d_success
 */
Status decode_data_from_image(uint size, char *data, FILE *fptr_src_image)
{
    char decoded_byte = 0;
    char encoded_data[MAX_ENC_IMAGE_BUF_SIZE];
    for (int i = 0; i < size; i++)
    {
        decoded_byte = 0;
        fread(encoded_data, sizeof(char), MAX_ENC_IMAGE_BUF_SIZE, fptr_src_image);
        decode_byte_from_lsb(&decoded_byte, encoded_data);
        data[i] = decoded_byte;
    }
    data[size] = '\0';
    return d_success;
}