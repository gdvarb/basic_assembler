#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 100

int print_line(FILE* fptr);
long int get_file_size(FILE* fptr);
void int_to_binary(long num, char *output, int bits);


int main()    
    {
        FILE* fptr;
        fptr = fopen("add.asm", "r");

        if (fptr == NULL)
        {
            printf("file was not opened\n");
        }
        else
        {
            printf("file was opened\n");
        }

        long int file_size = get_file_size(fptr);
        print_line(fptr);
        fclose(fptr);
        return 0;
    }

int print_line(FILE* fptr)
    {
        if (fptr == NULL)
        {
            printf("couldnt read file\n");
            return 1;
        }

        char data[MAX_LENGTH];
        printf("reading file:\n");
        while (fgets(data, MAX_LENGTH, fptr) != NULL)
        {
            char result[MAX_LENGTH];
            int write_index = 0;
            for (int i = 0; data[i] != '\0'; i++)
            {
                
                if (data[i] == '/' && data[i + 1] == '/')
                {
                    break;
                }

                if (data[i] == ' ' || data[i] == '\r' || data[i] == '\n')
                {
                    continue;
                }

                result[write_index] = data[i];
                write_index++;
            }
            
            result[write_index] = '\0';
            if (result[0] != '\0')
            {
                if (result[0] == '@')
                {
                    // A-instruction
                    char *slice = result + 1;
                    char *endptr;
                    int decimal_a_instruction = (int)strtol(slice, &endptr, 10);
                    // convert to binary
                    int total_bits = 16;
                    char binary_str[total_bits + 1];
                    int_to_binary(decimal_a_instruction, binary_str, total_bits);
                    printf("Binary A-instruction: %s\n", binary_str);
                    
                }
                else if (result[0] == '(')
                {
                    // todo: Handle label
                }
                else
                {
                    // handle c instruction
                    // locate =

                    char destination[25];
                    char *comp_ptr = strchr(result, '=');

                    
                    printf("Jump instruction: %s\n", jump_slice);

                    if (comp_ptr)
                    {
                        // Destination                        
                        size_t destination_length = comp_ptr - result;
                        memcpy(destination, result, destination_length);
                        destination[destination_length] = '\0';
                        printf("Destination: %s\n", destination);

                        
                        
                        char *comp_slice = comp_ptr + 1;
                        printf("Comp: %s\n", comp_slice);

                        //jump instruction
                        char *jump_ptr = strchr(comp_slice, ';');
                        char *jump_slice = jump_ptr + 1;


                    }
                            
                
                }
                printf("Result: %s\n", result);
            }
        }
        return 0;
    }

long int get_file_size(FILE* fptr)
    {
        if (fptr == NULL)
        {
            printf("couldnt get file size\n");
            return -1;
        }

        long starting_pos = ftell(fptr);
        
        fseek(fptr, 0L, SEEK_END);
        long int file_size= ftell(fptr);

        fseek(fptr, starting_pos, SEEK_SET);
        return file_size;


    }

void int_to_binary(long num, char *output, int bits)
{
    for (int i = bits - 1; i >= 0; i--)
    {
        output[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    output[bits] = '\0';
}




