#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LENGTH 100
#define TOTAL_BITS 17

struct SymbolEntry
{
    char name[100];
    int address;
};

struct SymbolEntry symbol_table[1000];
int table_counter = 0;
int rom_address = 0;
int custom_variable_address = 16;

long int get_file_size(FILE* fptr);
void int_to_binary(long num, char *output, int bits);
void add_symbol_entry(char *symbol_name, int memory_address);
int parse(FILE* fptr, int parse);


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
        int pass = 1;
        parse(fptr, pass);
        pass++;
        rewind(fptr);
        parse(fptr, pass);  
        fclose(fptr);
        return 0;
    }



int parse(FILE* fptr, int pass)
    {
        if (fptr == NULL)
        {
            printf("couldnt read file\n");
            return 1;
        }

        char data[MAX_LENGTH];
        while (fgets(data, MAX_LENGTH, fptr) != NULL)
        {
            char instruction[MAX_LENGTH];
            int instruction_idx = 0;
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
                instruction[instruction_idx] = data[i];
                instruction_idx++;
            }
            instruction[instruction_idx] = '\0';
            if (instruction[0] != '\0')
            {
                if (instruction[0] == '@')
                {   
                    switch (pass)
                    {
                        case 1:
                            rom_address++;
                            break;

                        case 2:
                            char binary_str[TOTAL_BITS];
                            char *symbol = instruction + 1;
                            char *end_ptr;
                            if (isalpha(symbol[0]) != 0)
                            {
                                printf("Symbol is %s\n", symbol);
                                int symbol_found = 0;
                                int address_to_convert = 0;
                                char symbol_str[TOTAL_BITS];
                                for (int i = 0; i < table_counter; i++)
                                {
                                    if (strcmp(symbol, symbol_table[i].name) == 0)
                                    {
                                        symbol_found = 1;
                                        address_to_convert = symbol_table[i].address;
                                    }
                                }
                                if (symbol_found == 0)
                                {
                                    //custom variable
                                    add_symbol_entry(symbol, custom_variable_address);
                                    address_to_convert = custom_variable_address;
                                    custom_variable_address++;
                                }
                                int_to_binary((long)address_to_convert, symbol_str, TOTAL_BITS);
                                printf("Binary instruction: %s\n", symbol_str);
                            }
                            else
                            {
                                int decimal_a_instruction = (int)strtol(instruction, &end_ptr, 10);
                                int_to_binary(decimal_a_instruction, binary_str, TOTAL_BITS);
                                printf("Binary A-Instruction: %s\n", binary_str);
                            }
                            break;
                    }
                }
                else 
                    // C-instruction
                    {
                        switch (pass)
                        {
                            case 1:
                                if (instruction[0] == '(')
                                {
                                    // add lable to the symbol table
                                    char *label_start = strchr(instruction, '(');
                                    label_start++;
                                    char *label_end = strchr(instruction, ')');
                                    size_t label_length = label_end - label_start;
                                    char label_definition[100];
                                    strncpy(label_definition, label_start, label_length);
                                    label_definition[label_length] = '\0';
                                    add_symbol_entry(label_definition, rom_address);
                                }
                                else
                                {
                                    rom_address++;
                                }
                                break;

                            case 2:
                                if (instruction[0] != '(')
                                {
                                    char dest[25];
                                    char comp[25];
                                    char jmp[25];

                                    char *comp_start = instruction;
                                    char *comp_ptr = strchr(instruction, '=');

                                    if (comp_ptr)
                                    {
                                        // extract destination
                                        comp_start = comp_ptr + 1;
                                        size_t dest_length = comp_ptr - instruction;
                                        memcpy(dest, instruction, dest_length);
                                        dest[dest_length] = '\0';
                                        printf("Destination: %s\n", dest);
                                    }

                                    char *jmp_ptr = strchr(comp_start, ';');
                                    if (jmp_ptr)
                                    {
                                        //extract comp and jmp
                                        size_t comp_length = jmp_ptr - comp_start;
                                        memcpy(comp, comp_start, comp_length);
                                        comp[comp_length] = '\0';

                                        strcpy(jmp, jmp_ptr + 1);
                                    }
                                    else
                                    {
                                        //only comp portion
                                        strcpy(comp, comp_start);
                                    }
                                }
                                break;
                        }
                    }
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


void add_symbol_entry(char *symbol_name, int memory_address)
{
    strcpy(symbol_table[table_counter].name, symbol_name);
    symbol_table[table_counter].address = memory_address;
    table_counter++;
}