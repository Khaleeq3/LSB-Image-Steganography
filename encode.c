#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/*  Check operation type
 *  Input: command line arguments
 *  Output: Operation type
 *  Description: Checks the 2nd argument is a valid option or not
 *  Return value: e_encode, e_decode, e_unsupported
 */
OperationType check_operation_type(char *argv[])
{

    if (!(strcmp(argv[1], "-e")))
    {
        return e_encode;
    }
    else if (!(strcmp(argv[1], "-d")))
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
    
}

/* Read and validate encode arguments
 * Input: command line arguments and address of structure variable which holds encoding data
 * Output: Valid data for encooding
 * Desciption: Validates and read the input data for encoding
 * return value: e_success, e_failure
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{   
    /* Check if the sufficient arguments are passed */
    if (argv[2] == NULL || argv[3] == NULL) 
    {
        puts("ERROR: Insufficient arguments for encoding.");
        puts("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]");
        return e_failure;
    }

    char *str;  /*temporary vaiable to check the extension */
    /* Do error handling for source image file */
    if((str = strstr(argv[2], ".bmp")) == NULL)
    {
        puts("ERROR: Unsupported format of image file");
        puts("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]");
        return e_failure;
    }
    else
    {
        encInfo->src_image_fname = argv[2];
    }

    /* Do error handling for secret data file */
    if ((str = strchr(argv[3], '.')) == NULL || (strcmp(str, ".txt")) && (strcmp(str, ".c")) && (strcmp(str, ".sh")))
    {
        puts("ERROR: Unsupported format of secret file.");
        puts("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]");
        return e_failure;
    }
    else
    {
        encInfo->secret_fname = argv[3];
    }

    /* Create Output file*/
    if(argv[4] == NULL)
    {
        printf("INFO: Output file not mentioned. Creating steged_img.bmp as default\n");
        encInfo->stego_image_fname = "steged_img.bmp";
        return e_success;
    }
    else if((str = strchr(argv[4], '.')) == NULL)
    {
        strcat(argv[4], ".bmp");
    }
    else
    {
        strcpy(str, ".bmp");
    }

    encInfo->stego_image_fname = argv[4];

    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    // printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    // printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");

    // Open Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->src_image_fname);

    // Open Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->secret_fname);

    // Open Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");

    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->stego_image_fname);

    // No failure return e_success
    return e_success;
}

/* Do Encoding
 * Input: address of structure varible which holds encoding data 
 * Output: Encoded image 
 * Description: Encodes the secret file data into the image 
 * return value: e_success, e_failure
 */
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: ## Encoding Procedure Started ##\n");

    // get the size of secret file
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);
    if ((encInfo->size_secret_file = get_file_size(encInfo->fptr_secret)) == 0)
    {
        printf("%s file is empty\n", encInfo->secret_fname);
        return e_failure;
    }
    else
    {
        printf("INFO: Done. Not Empty\n");
    }

    // Check capacity of source image to handle the secret data
    printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
    if(check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: %s file doesn't have sufficient capacity to encode the secret data\n", encInfo->src_image_fname);
        return e_failure;
    }
    else
    {
        printf("INFO: Done. Found OK\n");
    }

    // Copy header of bmp file
    printf("INFO: Copying Image header\n");
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: copy_bmp_header function failed\n");
        return e_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }
    
    // Encode Magic String
    printf("INFO: Encoding Magic String Signature\n");
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR: encode_magic_string function is failed\n");
        return e_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }
    
    // Encoding secret file extension

    // Copy secret file extension
    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));

    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: encode_secret_file_extn function failed\n");
        return e_failure;
    }
    else
    {
        printf("INFO: Done\n");
    }

    // Error handling for Encode secret file size
    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
    {
        printf("INFO: Done\n");
    }
    else
    {
        printf("ERROR: encode_secret_file_size function is failed\n");
        return e_failure;
    }

    // Error handling for Encode secret file data
    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    if(encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO: Done\n");
    }
    else
    {
        printf("ERROR: encode_secret_file_data function is failed\n");
        return e_failure;
    }


    // Error handling for Encode remaining data
    printf("INFO: Copying Left Over Data\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO: Done\n");
    }
    else
    {
        printf("INFO: ERROR: copy_remaining_ing_data function is failed\n");
        return e_failure;
    }

    // Close all opened files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
    
    // Encoding done
    return e_success;
}

/* Check Capacity 
 * Input: Address of structure variable which holds the encoding data
 * Output: Image size
 * Description: Checks the image capacity to handle secret data
 * return value: e_success, e_failure
 */
Status check_capacity(EncodeInfo *encInfo)
{
    // Get the size of source image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    
    // Check capacity
    if (encInfo->image_capacity > 54 + ( strlen(MAGIC_STRING) + 4 + strlen(".txt") + 4 + encInfo->size_secret_file) * 8)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
    
}

/* get file size 
 * Input: File pointer
 * Output: Size of file
 * Description: Finds the size of the file
 * Return value: Size of file
 */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}


/* encode magic string
 * Input: Magic string and address of structure vaiable which holds the encoding data
 * Output: Image encoded with magic string
 * Description: Encodes the magic string to output image
 * Return value: e_success, e_failure
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Error handling for encoding magic string into stego image
    if(encode_data_to_image((char *) magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: encode_data_to_image function is failed\n");
        return e_failure;
    }

    return e_success;
    
}

/* Encode data to image
 * Input: Data to be encoded and its size. source image and stego image file pointers
 * Output: Output image which encoded data
 * Description: Encodeds the provided data to image
 * Return value: e_success
 */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // Temporary array get the RGB data of source image
    char image_buffer[MAX_IMAGE_BUF_SIZE];

    // Read 8 bytes from source image each time to encode 1 byte of data and store the encoded data to stego image
    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE, fptr_src_image);
        encode_byte_to_lsb(data[i], image_buffer);
        fwrite(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE, fptr_stego_image);
    }

    return e_success;

}

/* Encode Secret file data 
 * Input: Address of structure variable which holds encoding data 
 * Output: Image with Secret data encoded in it
 * Description: Encodes the content of secret file into stego image
 * Return value: e_success
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Get the secret file pointer to starting position
    rewind(encInfo->fptr_secret);

    // Get data byte by byte from secret file and encode in stego image
    while (!feof(encInfo->fptr_secret))
    {
        fread(encInfo->secret_data, sizeof(char), MAX_SECRET_BUF_SIZE, encInfo->fptr_secret);
        encode_data_to_image(encInfo->secret_data, MAX_SECRET_BUF_SIZE, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    }

    return e_success;
}

/* Encode secret file size
 * Input: file size, Structre variable which holds the encoding data 
 * Output: Stego image with size encoded in it
 * Description: Encode the given file size to stego image
 * Return value: e_success
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[MAX_IMAGE_BUF_SIZE * 4];  // To encode 32 bits(4 bytes) we need 32 bytes

    // Read 32 bytes of RGB data from source image and encode it with secret data and store in stego image
    fread(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE * 4, encInfo->fptr_src_image);
    encode_size_to_lsb(encInfo->size_secret_file, image_buffer);
    fwrite(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE * 4, encInfo->fptr_stego_image);

    return e_success;
}

/* Copy bmp header
 * Input: Source image and stego image file pointers
 * Output: Stego image with same bmp header as source image header
 * Description: Copies the source image bmp header to stego image
 * return value: e_success
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54] = {0};
    rewind(fptr_src_image);
    if(fread(header, sizeof(char), 54, fptr_src_image) != 54) return e_failure;
    if(fwrite(header, sizeof(char), 54, fptr_dest_image) != 54) return e_failure;

    return e_success;
}

/* encode byte to lsb's of RGB data of image
 * Input: One byte of data to be encoded and image buffer where the data is to be encoded
 * Output: Encoded image buffer
 * Description: Encode the one byte of data to the lsb's of RGB data of image
 * Return value: e_success
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 7, index = 0; i >= 0; index++, i--)
    {
        image_buffer[index] = (image_buffer[index] & ~1) | ((unsigned) data >> i & 1);
    }
    return e_success;
}

/* Encodes the size to lsb of RGB data
 * Input: size to be encoded and image buffer where data is to be encoded
 * Output: Encoded image buffer with given size
 * Description: Encodes the given size to the lsb's of RGB data
 * Return value: e_success
 */
Status encode_size_to_lsb(int size, char *image_buffer)
{
    for (int i = 31, index = 0; i >= 0; index++, i--)
    {
        image_buffer[index] = (image_buffer[index] & ~1) |  ((unsigned) size >> i & 1);
    }
    return e_success;
}

/* Encode secret file extension
 * Input: file extension and Address of structure variable which holds the encoding data
 * Output: Stego image with encoded file extension
 * Description: Encode the secret file extension to stego image
 * Return value: e_success, e_failure
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
   int size_secret_extn = strlen(file_extn);
   char image_buffer[MAX_IMAGE_BUF_SIZE * 4];

    // Encode secret file extension size
    fread(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE * 4, encInfo->fptr_src_image);
    encode_size_to_lsb(size_secret_extn, image_buffer);
    fwrite(image_buffer, sizeof(char), MAX_IMAGE_BUF_SIZE * 4, encInfo->fptr_stego_image);

    // Encode secret file extension
    if (encode_data_to_image(encInfo->extn_secret_file, size_secret_extn, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Copy left over data
 * Input: Source image and stego image file pointers
 * Output: Completely encoded stego image
 * Description: Encode the left over data from source image to stego image
 * Return value: e_success
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char temp;
    while (fread(&temp, sizeof(char), 1, fptr_src) == 1)
    {
        if(fwrite(&temp, sizeof(char), 1, fptr_dest) != 1) return e_failure;
    }
    
    return e_success;
}
