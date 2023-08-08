#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cstdio>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sched.h>
#include <sys/time.h>
#include <fcntl.h>
#include <iostream>
#include <atomic>

#define MAX_LINES 10000000
#define MAX_LINE_LENGTH 100

// Define the structure to store opcode and key
struct Data {
    uint8_t opcode;
    uint64_t key;
}__attribute__((packed));

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Open the input file for reading
    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Error opening input file: %s\n", argv[1]);
        return 1;
    }

    // Create an array to store the data
    struct Data* data_array = (struct Data*)malloc(sizeof(struct Data) * MAX_LINES);
    int num_lines = 0;

    // Read lines from the CSV file and store them in the array
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, input_file) != NULL) {
	if(line[0] != 'R') {
		continue;
	}
        if (num_lines >= MAX_LINES) {
            printf("Maximum number of lines reached. Exiting...\n");
            break;
        }

        // Parse the line and extract the opcode and key
	char opcode_str[10];
        uint8_t opcode;
        uint64_t key;
        sscanf(line, "%s user%lu\n", opcode_str, &key);
        if (strncmp(opcode_str, "READ", 4) == 0) {
		opcode = 1;
	}
	printf("opcode and key is: %hhu %lu\n", opcode, key);

        // Store the data in the array
        data_array[num_lines].opcode = opcode;
        data_array[num_lines].key = key;
        num_lines++;
    }

    // Close the input file
    fclose(input_file);

    // Open the output file for writing
    FILE* output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        printf("Error opening output file: %s\n", argv[2]);
        return 1;
    }

    // Write the data from the array to the output file
    for (int i = 0; i < num_lines; i++) {
        //fprintf(output_file, "%hhu%lu", data_array[i].opcode, data_array[i].key);
        fwrite(&data_array[i].opcode, sizeof(uint8_t), 1, output_file);
        fwrite(&data_array[i].key, sizeof(uint64_t), 1, output_file);
    }

    // Close the output file
    fclose(output_file);

    int test_file = open(argv[2], O_RDONLY); 
    if (test_file < 0) {
        printf("Can not open %s\n", argv[2]);
        return NULL;
    }
    struct stat st;
    fstat(test_file, &st);
    int file_size = st.st_size;
    int num_op = file_size / sizeof(struct Data);

    printf("File size: %d, num ops: %d\n", file_size, num_op);
    printf("Size of struct: %u\n", sizeof(struct Data));

    struct Data* oplist = (struct Data *)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, test_file, 0);
    if (oplist == MAP_FAILED) {
        printf("Can not mmap data_buf\n");
        return NULL;
    }
    for(uint64_t i = 0; i < num_op; i++) {
    	printf("Opcode and key is: %d, %lu\n", oplist[i].opcode, oplist[i].key);
    }
    close(test_file);

    printf("Data successfully loaded from %s and written to %s.\n", argv[1], argv[2]);

    free(data_array);
    return 0;
}
