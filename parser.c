#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LENGTH 100
#define TOTAL_BITS 17
#define COMP_TABLE_LENGTH 18
#define JMP_TABLE_LENGTH 8

struct SymbolEntry
{
    char name[100];
    int address;
};
struct SymbolEntry symbol_table[1000];
int table_counter = 0;


struct JmpEntry
{
    char command[5];
    char bits[4];
};
struct JmpEntry jmp_table[8] =
{
    {"null", "000"},
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"}
};

struct CompEntry
{
    char command[4];
    char bits[7];
};
struct CompEntry comp_table[18] =
{
    {"0", "101010"},
    {"1", "111111"},
    {"-1", "111010"},
    {"D", "001100"},
    {"A", "110000"},
    {"!D", "001101"},
    {"!A", "110001"},
    {"-D", "001111"},
    {"-A", "110011"},
    {"D+1", "011111"},
    {"A+1", "110111"},
    {"D-1", "001110"},
    {"A-1", "110010"},
    {"D+A", "000010"},
    {"D-A", "010011"},
    {"A-D", "000111"},
    {"D&A", "000000"},
    {"D|A", "010101"}
};

int rom_address = 0;
int custom_variable_address = 16;

long int get_file_size(FILE* fptr);
void int_to_binary(long num, char *output, int bits);
void add_symbol_entry(char *symbol_name, int memory_address);
int parse(FILE* fptr, int parse, FILE* wptr);
char* dest_bits(char* dest);
char* comp_bits(char *comp_inst);
char* jmp_bits(char *jmp_inst);



int main()    
    {
        FILE* fptr;
        FILE* wptr;
        fptr = fopen("add.asm", "r");
        wptr = fopen("prog.hack", "w");

        if (fptr == NULL)
        {
            printf("file was not opened\n");
        }
        else
        {
            printf("file was opened\n");
        }

        //long int file_size = get_file_size(fptr);
        int pass = 1;
        parse(fptr, pass, wptr);
        pass++;
        rewind(fptr);
        parse(fptr, pass, wptr);  
        fclose(fptr);
        fclose(wptr);
        return 0;
    }



int parse(FILE* fptr, int pass, FILE* wptr)
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
                                //printf("Symbol is %s\n", symbol);
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
                                int_to_binary((long)address_to_convert, symbol_str, TOTAL_BITS - 1);
                                //printf("Binary instruction: %s\n", symbol_str);
                                fprintf(wptr, "%s\n", symbol_str);
                            }
                            else
                            {
                                int decimal_a_instruction = (int)strtol(instruction, &end_ptr, 10);
                                int_to_binary(decimal_a_instruction, binary_str, TOTAL_BITS - 1);
                                //printf("Binary A-Instruction: %s\n", binary_str);
                                fprintf(wptr, "%s", binary_str);
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
                                    char instruction_bits[TOTAL_BITS];
                                    char dest[25];
                                    char comp[25];
                                    char jmp[25];
                                    
                                    char d_bits[4] = "000";
                                    char j_bits[4] = "000";
                                    char c_bits[8] = "0000000";

                                    char *comp_start = instruction;
                                    char *comp_ptr = strchr(instruction, '=');

                                    if (comp_ptr)
                                    {
                                        // extract destination
                                        comp_start = comp_ptr + 1;
                                        size_t dest_length = comp_ptr - instruction;
                                        memcpy(dest, instruction, dest_length);
                                        dest[dest_length] = '\0';

                                        char *dest_result = dest_bits(dest);
                                        strcpy(d_bits, dest_result);
                                        //printf("Destination: %s\n", dest);
                                        free(dest_result);
                                    }

                                    char *jmp_ptr = strchr(comp_start, ';');
                                    if (jmp_ptr)
                                    {
                                        //extract comp and jmp
                                        size_t comp_length = jmp_ptr - comp_start;
                                        memcpy(comp, comp_start, comp_length);
                                        comp[comp_length] = '\0';
                                        char *comp_result = comp_bits(comp);
                                        strcpy(c_bits, comp_result);
                                        free(comp_result);


                                        strcpy(jmp, jmp_ptr + 1);
                                        char *jmp_result = jmp_bits(jmp);
                                        strcpy(j_bits, jmp_result);
                                    }
                                    else
                                    {
                                        //only comp portion
                                        strcpy(comp, comp_start);
                                        char *comp_result = comp_bits(comp);
                                        strcpy(c_bits, comp_result);
                                        free(comp_result);

                                    }
                                    snprintf(instruction_bits, sizeof(instruction_bits), "111%s%s%s", c_bits, d_bits, j_bits);
                                    printf("Instruction_bits: %s\n", instruction_bits);
                                    fprintf(wptr, "%s\n", instruction_bits);
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

char* dest_bits(char *dest)
{
    char bits[4] = "000";
    if (strchr(dest, 'A')) bits[0] = '1';
    if (strchr(dest, 'D')) bits[1] = '1';
    if (strchr(dest, 'M')) bits[2] = '1';

    size_t dest_bits_len = strlen(bits);
    char* dest_bits = malloc(dest_bits_len + 1);
    memcpy(dest_bits, bits, dest_bits_len);
    dest_bits[dest_bits_len] = '\0';
    return dest_bits;
}

char* comp_bits(char *comp_inst)
{
    char comp_key[4];
    strcpy(comp_key, comp_inst);
    char a_bit = '0';
    char *m_pos = strchr(comp_key, 'M');
    if (m_pos != NULL)
    {
        a_bit = '1';
        *m_pos = 'A';
    }

    for (int i = 0; i < COMP_TABLE_LENGTH; i++)
    {
        if (strcmp(comp_key, comp_table[i].command) == 0)
        {
            size_t comp_bits_len = strlen(comp_table[i].bits);
            char *comp_bits = malloc(comp_bits_len + 2);
            if (comp_bits == NULL) return NULL;
            comp_bits[0] = a_bit;
            memcpy(comp_bits + 1, comp_table[i].bits, comp_bits_len);
            comp_bits[comp_bits_len + 1] = '\0';
            return comp_bits;
        }
    }
    return NULL;
}

char* jmp_bits(char *jmp_inst)
{
    for (int i = 0; i < JMP_TABLE_LENGTH; i++)
    {
        if (strcmp(jmp_table[i].command, jmp_inst) == 0)
        {
            return jmp_table[i].bits;
        }
    }
    return NULL;
}