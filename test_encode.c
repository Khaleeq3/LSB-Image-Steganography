/*
Name: Khaleeque Mujtahid
Date: 20/12/2023
Description: Digital image steganography.
Sample Execution:
Encoding:   
         ./a.out -e beautiful.bmp secret.txt output
        INFO: Opening required files
        INFO: Opened beautiful.bmp
        INFO: Opened secret.txt
        INFO: Opened output.bmp
        INFO: Done
        INFO: ## Encoding Procedure Started ##
        INFO: Checking for secret.txt size
        INFO: Done. Not Empty
        INFO: Checking for beautiful.bmp capacity to handle secret.txt
        INFO: Done. Found OK
        INFO: Copying Image header
        INFO: Done
        INFO: Encoding Magic String Signature
        INFO: Done
        INFO: Encoding secret.txt File Extension
        INFO: Done
        INFO: Encoding secret.txt File Size
        INFO: Done
        INFO: Encoding secret.txt File Data
        INFO: Done
        INFO: Copying Left Over Data
        INFO: Done
        INFO: ## Encoding Done Seccessfully ##

Decoding:
        ./a.out -d output.bmp output
        INFO: ## Decoding Procedure Started ##
        INFO: Opening required files
        INFO: Opened output.bmp
        INFO: Decodeing Magic String Siganture
        INFO: Done
        INFO: Decoding output file extension
        INFO: Done
        INFO: Opened output.txt
        INFO: Done. Opened all required files
        INFO: Decoding output.txt File Size
        INFO: Done
        INFO: Decoding output.txt File Data
        INFO: Done
        INFO: ## Decoding Done Successfully ##
*/

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    /* Declare a structure variable to store encoding data */
    EncodeInfo encInfo;
    /* uint img_size; */

    /* Declare a structure variable to store decoding data */
    DecodeInfo decInfo;
    
    /*
    // Fill with sample filenames
    encInfo.src_image_fname = "beautiful.bmp";
    encInfo.secret_fname = "secret.txt";
    encInfo.stego_image_fname = "stego_img.bmp";
    */

    /* Check if the user has passed any option for operation */
    if(argv[1] == NULL)
    {
        puts("ERROR: Insufficient arguments");
        puts("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]");
        puts("Usage: ./a.out -d <.bmp_file> [output file]");
        return 1;
    }

    /* Do error handling for check operation */
    if (check_operation_type(argv) == e_encode)
    {
        /* Read and do error handling for encode arguments */
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            puts("ERROR: Read and validate function failed");
            return 1;
        }
        
        /* Do error handling for file openings */
        if (open_files(&encInfo) == e_failure)
        {
        	printf("ERROR: %s function failed\n", "open_files" );
        	return 1;
        }
        else
        {
        	// printf("SUCCESS: %s function completed\n", "open_files" );
            printf("INFO: Done\n");
        }

        /* Do error handling for encoding */
        if((do_encoding(&encInfo)) ==  e_failure)
        {
            printf("ERROR: do_encoding function failed\n");
            return 1;
        }
        else
        {
           printf("INFO: ## Encoding Done Seccessfully ##\n");
        }
    }
    else if (check_operation_type(argv) == e_decode)
    {
        /* Do Error handling for decode arguments */
        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("ERROR: read_and_validate function failed\n");
            return 1;
        }

        // do error handling for decoding
        if (do_decoding(&decInfo) == e_failure)
        {
            printf("INFO: do_decoding function failed\n");
            return 1;
        }
        else
        {
            printf("INFO: ## Decoding Done Successfully ##\n");
        }
    }
    else
    {
        puts("ERROR: Invalid Operation");
        puts("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]");
        puts("Usage: ./a.out -d <.bmp_file> [output file]");
        return 1;
    }
    /*
    // Test get_image_size_for_bmp
    img_size = get_image_size_for_bmp(encInfo.fptr_src_image);
    printf("INFO: Image size = %u\n", img_size);
    */
    return 0;
}
