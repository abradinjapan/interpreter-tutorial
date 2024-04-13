#ifndef ANVIL__alloy
#define ANVIL__alloy

/* Include */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/* Define */
typedef uint8_t ANVIL__u8;
typedef uint16_t ANVIL__u16;
typedef uint32_t ANVIL__u32;
typedef uint64_t ANVIL__u64;
typedef int8_t ANVIL__s8;
typedef int16_t ANVIL__s16;
typedef int32_t ANVIL__s32;
typedef int64_t ANVIL__s64;

typedef ANVIL__u8 u8;
typedef ANVIL__u16 u16;
typedef ANVIL__u32 u32;
typedef ANVIL__u64 u64;
typedef ANVIL__s8 s8;
typedef ANVIL__s16 s16;
typedef ANVIL__s32 s32;
typedef ANVIL__s64 s64;

typedef void* ANVIL__address;

typedef ANVIL__u64 ANVIL__length;

typedef ANVIL__u64 ANVIL__register_integer_value;

typedef ANVIL__u64 ANVIL__instruction_count;

typedef ANVIL__u8 ANVIL__character;

#define ANVIL__unused_register_ID 0
#define ANVIL__define__bits_in_byte 8
#define ANVIL__define__run_forever (ANVIL__u64)-1
#define ANVIL__define__input_string_max_length 256

// boolean type
typedef enum ANVIL__bt {
	ANVIL__bt__false = 0,
	ANVIL__bt__true = 1,
	ANVIL__bt__COUNT = 2,
} ANVIL__bt;

/* Allocation */
// return memory to OS
void ANVIL__close__allocation(ANVIL__address start, ANVIL__address end) {
	// do useless operation to quiet compiler
	end = end;

	// return memory
	free(start);

	return;
}

// ask OS for memory
ANVIL__address ANVIL__open__allocation(ANVIL__length length) {
	// return allocation
	return (ANVIL__address)malloc(length);
}

/* Buffer */
// buffer
typedef struct __attribute__((packed)) ANVIL__buffer {
	ANVIL__address start;
	ANVIL__address end;
} ANVIL__buffer;

// close buffer
void ANVIL__close__buffer(ANVIL__buffer buffer) {
	// close allocation
	ANVIL__close__allocation(buffer.start, buffer.end);

	return;
}

// create buffer to have specific exact contents
ANVIL__buffer ANVIL__create__buffer(ANVIL__address start, ANVIL__address end) {
	ANVIL__buffer output;

	// setup output
	output.start = start;
	output.end = end;

	return output;
}

// create buffer in it's standard null setup
ANVIL__buffer ANVIL__create_null__buffer() {
	// return standard null buffer
	return ANVIL__create__buffer(0, 0);
}

// calculate buffer length
ANVIL__length ANVIL__calculate__buffer_length(ANVIL__buffer buffer) {
    return (ANVIL__length)((u8*)buffer.end - (u8*)buffer.start) + 1;
}

// open buffer
ANVIL__buffer ANVIL__open__buffer(ANVIL__length length) {
	ANVIL__buffer output;

	// attempt allocation
	output.start = ANVIL__open__allocation(length);

	// set end of buffer according to allocation success
	if (output.start != 0) {
		output.end = (ANVIL__address)((((ANVIL__u64)output.start) + length) - 1);
	} else {
		output.end = 0;
	}

	return output;
}

// create or open a buffer from a string literal (can either duplicate buffer or simply reference original) (can opt out of null termination)
ANVIL__buffer ANVIL__open__buffer_from_string(u8* string, ANVIL__bt duplicate, ANVIL__bt null_terminate) {
    ANVIL__buffer output;
    ANVIL__length length;

    // setup length
    length = 0;

    // get buffer length
    while (string[length] != 0) {
        length++;
    }

    // optionally append null termination
    if (null_terminate == ANVIL__bt__true) {
        length++;
    }

    // reference or duplicate
    if (duplicate == ANVIL__bt__true) {
        // attempt allocation
        output = ANVIL__open__buffer(length);

        // check for null allocation
        if (output.start == 0) {
            // return empty buffer
            return output;
        }

        // copy buffer byte by byte
        for (ANVIL__length byte_index = 0; byte_index < length; byte_index++) {
            // copy byte
            ((ANVIL__u8*)output.start)[byte_index] = string[byte_index];
        }
    } else {
        // setup duplicate output
        output.start = string;
        output.end = string + length - 1;
    }

    return output;
}

// read buffer
ANVIL__u64 ANVIL__read__buffer(ANVIL__address source, ANVIL__length byte_amount) {
	ANVIL__u64 output;

	// setup output
	output = 0;

	// read buffer
	for (ANVIL__u64 byte_index = 0; byte_index < byte_amount; byte_index += 1) {
		// get byte
		((ANVIL__u8*)&output)[byte_index] = ((ANVIL__u8*)source)[byte_index];
	}

	// return output
	return output;
}

// write buffer
void ANVIL__write__buffer(ANVIL__u64 source, ANVIL__length byte_amount, ANVIL__address destination) {
	// write data to buffer
	for (ANVIL__length byte_index = 0; byte_index < byte_amount; byte_index += 1) {
		// write byte
		((ANVIL__u8*)destination)[byte_index] = ((ANVIL__u8*)&source)[byte_index];
	}
	
	return;
}

// create buffer from file
ANVIL__buffer ANVIL__move__file_to_buffer(ANVIL__buffer null_terminated_file_name) {
	ANVIL__buffer output;
	FILE* file_handle;
	ANVIL__u64 file_size;

	// open file
	file_handle = fopen((const char*)null_terminated_file_name.start, "rb");

	// check if the file opened
	if (file_handle == 0) {
		// if not, return empty buffer
		return ANVIL__create_null__buffer();
	}

	// get file size
	fseek(file_handle, 0, SEEK_END);
	file_size = ftell(file_handle);
	fseek(file_handle, 0, SEEK_SET);

	// allocate buffer
	output = ANVIL__open__buffer(file_size);

	// check if buffer allocated
	if (output.start == 0) {
		// close file handle
		fclose(file_handle);

		// return empty buffer
		return output;
	}

	// read file into buffer
	fread(output.start, file_size, 1, file_handle);

	// close file handle
	fclose(file_handle);

	// return buffer
	return output;
}

// create file from buffer
void ANVIL__move__buffer_to_file(ANVIL__bt* error, ANVIL__buffer null_terminated_file_name, ANVIL__buffer data) {
	FILE* file_handle;

    // setup error to no error to start
    *error = ANVIL__bt__false;

	// open file
	file_handle = fopen((const char*)null_terminated_file_name.start, "w+b");

	// check if the file opened
	if (file_handle == 0) {
		// if not, return error
        *error = ANVIL__bt__true;

		return;
	}

	// write buffer to file
	fwrite(data.start, ANVIL__calculate__buffer_length(data), 1, file_handle);

	// close file handle
	fclose(file_handle);

	// return
	return;
}

// print buffer
void ANVIL__print__buffer(ANVIL__buffer buffer) {
    // print character by character
    for (ANVIL__address character = buffer.start; character <= buffer.end; character += sizeof(ANVIL__character)) {
        // print character
        putchar(*(ANVIL__character*)character);
    }

    return;
}

/* Machine Specifications */
typedef ANVIL__address ANVIL__register;
typedef ANVIL__u8 ANVIL__instruction_ID;
typedef ANVIL__u8 ANVIL__flag_ID;
typedef ANVIL__u8 ANVIL__operation_ID;
typedef ANVIL__u8 ANVIL__register_ID;
typedef ANVIL__u64 ANVIL__bit_count;
typedef ANVIL__u64 ANVIL__byte_count;

// scraplet types
typedef enum ANVIL__st {
    // scraplet types
    ANVIL__st__instruction_ID,
    ANVIL__st__flag_ID,
    ANVIL__st__operation_ID,
    ANVIL__st__register_ID,
    ANVIL__st__register,

    // count
    ANVIL__st__COUNT,
} ANVIL__st;

// should the next instruction process
// next instruction type
typedef enum ANVIL__nit {
    // next instruction types
    ANVIL__nit__return_context,
    ANVIL__nit__next_instruction,

    // count
    ANVIL__nit__COUNT,
} ANVIL__nit;

/* Alloy Specification */
// context register organization defines
// register type
typedef enum ANVIL__rt {
    // start of defined values
    ANVIL__rt__START = 0,

    // basic registers
    ANVIL__rt__program_start_address = ANVIL__rt__START,
    ANVIL__rt__program_current_address,
    ANVIL__rt__program_end_address,
    ANVIL__rt__error_code,
    ANVIL__rt__flags_0,
    ANVIL__rt__flags_1,
    ANVIL__rt__flags_2,
    ANVIL__rt__flags_3,

    // end of defined registers
    ANVIL__rt__END,

    // count
    ANVIL__rt__RESERVED_COUNT = ANVIL__rt__END - ANVIL__rt__START,

    // statistics
    ANVIL__rt__FIRST_ID = ANVIL__rt__START,
    ANVIL__rt__LAST_ID = 255, // NUMBER DOES NOT CHANGE!
    ANVIL__rt__TOTAL_COUNT = ANVIL__rt__LAST_ID + 1,
} ANVIL__rt;

// context
typedef struct ANVIL__context {
    ANVIL__register registers[ANVIL__rt__TOTAL_COUNT];
} ANVIL__context;

// instruction type and instruction ID defines (type number is also ID)
typedef enum ANVIL__it {
    // start of defined instructions
    ANVIL__it__START = 0,

    // defined instructions
    ANVIL__it__stop = ANVIL__it__START, // returns context to caller
    ANVIL__it__write_register, // overwrites entire register with hard coded value
    ANVIL__it__operate, // inter-register operations
    ANVIL__it__request_memory, // allocate request
    ANVIL__it__return_memory, // deallocate request
    ANVIL__it__address_to_register, // read memory into register
    ANVIL__it__register_to_address, // write register to memory
    ANVIL__it__file_to_buffer, // get file into buffer
    ANVIL__it__buffer_to_file, // write buffer to disk

    // extra defined instructions
    ANVIL__it__debug__putchar, // print one character to stdout
    ANVIL__it__debug__print_register_as_decimal, // print an entire register as a decimal number
    ANVIL__it__debug__fgets, // read one line from stdin
    ANVIL__it__debug__mark_data_section, // mark a section of data (NOP)
    ANVIL__it__debug__mark_code_section, // mark a section of code (NOP)

    // end of defined instruction types
    ANVIL__it__END,

    // count
    ANVIL__it__COUNT = ANVIL__it__END - ANVIL__it__START,
} ANVIL__it;

// instruction length types
typedef enum ANVIL__ilt {
    ANVIL__ilt__stop = sizeof(ANVIL__instruction_ID),
    ANVIL__ilt__write_register = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__register) + sizeof(ANVIL__register_ID),
    ANVIL__ilt__operate = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__flag_ID) + sizeof(ANVIL__operation_ID) + (sizeof(ANVIL__register_ID) * 4),
    ANVIL__ilt__request_memory = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 3),
    ANVIL__ilt__return_memory = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 2),
    ANVIL__ilt__address_to_register = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__flag_ID) + (sizeof(ANVIL__register_ID) * 3),
    ANVIL__ilt__register_to_address = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__flag_ID) + (sizeof(ANVIL__register_ID) * 3),
    ANVIL__ilt__file_to_buffer = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 4),
    ANVIL__ilt__buffer_to_file = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 4),
    ANVIL__ilt__run = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 3),
    ANVIL__ilt__debug__putchar = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__register_ID),
    ANVIL__ilt__debug__print_register_as_decimal = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__register_ID),
    ANVIL__ilt__debug__fgets = sizeof(ANVIL__instruction_ID) + (sizeof(ANVIL__register_ID) * 2),
    ANVIL__ilt__debug__mark_data_section = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__register),
    ANVIL__ilt__debug__mark_code_section = sizeof(ANVIL__instruction_ID) + sizeof(ANVIL__register),
} ANVIL__ilt;

// error codes
typedef enum ANVIL__et {
    // error codes
    ANVIL__et__no_error = 0,
    ANVIL__et__invalid_instruction_ID,
    ANVIL__et__divide_by_zero,
    ANVIL__et__modulous_by_zero,
    ANVIL__et__file_not_found,
    ANVIL__et__file_not_created,
    ANVIL__et__invalid_address__address_to_register,
    ANVIL__et__invalid_address__register_to_address,

    // count
    ANVIL__et__COUNT,
} ANVIL__et;

// operation types
typedef enum ANVIL__ot {
    // copy
    ANVIL__ot__register_to_register, // copies one register to another without transformation
    ANVIL__ot__fetch_register, // find register dynamically by ID inside another register and copy it's value

    // binary operations
    ANVIL__ot__bits_or,
    ANVIL__ot__bits_invert,
    ANVIL__ot__bits_and,
    ANVIL__ot__bits_xor,
    ANVIL__ot__bits_shift_higher,
    ANVIL__ot__bits_shift_lower,
    ANVIL__ot__bits_overwrite,

    // arithmetic operations
    ANVIL__ot__integer_add,
    ANVIL__ot__integer_subtract,
    ANVIL__ot__integer_multiply,
    ANVIL__ot__integer_division,
    ANVIL__ot__integer_modulous,

    // comparison operations
    ANVIL__ot__integer_within_range, // equivalent to (range_start <= integer_n <= range_end) -> boolean

    // flag operations
    ANVIL__ot__flag_or,
    ANVIL__ot__flag_invert,
    ANVIL__ot__flag_and,
    ANVIL__ot__flag_xor,
    ANVIL__ot__flag_get,
    ANVIL__ot__flag_set,

    // count
    ANVIL__ot__COUNT,
} ANVIL__ot;

/* Helper Functions */
// get a pointer from a context to a register inside that context
ANVIL__register* ANVIL__get__register_address_from_context(ANVIL__context* context, ANVIL__register_ID register_ID) {
    // return data
    return &((*context).registers[register_ID]);
}

// get register value from context
ANVIL__register ANVIL__get__register_from_context(ANVIL__context* context, ANVIL__register_ID register_ID) {
    // return data
    return *ANVIL__get__register_address_from_context(context, register_ID);
}

// set register value by pointer
void ANVIL__set__register_by_address(ANVIL__register* destination_register, ANVIL__register value) {
    // set value
    *destination_register = value;

    return;
}

// set value in error code register
void ANVIL__set__error_code_register(ANVIL__context* context, ANVIL__et error_code) {
    // set error
    ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, ANVIL__rt__error_code), (ANVIL__register)error_code);

    return;
}

// get flag (value)
ANVIL__bt ANVIL__get__flag_from_context(ANVIL__context* context, ANVIL__flag_ID flag_ID) {
    ANVIL__u8 masks[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

    // return bit
    return (ANVIL__bt)((((ANVIL__u8*)&((*context).registers[ANVIL__rt__flags_0]))[flag_ID / 8] & masks[flag_ID % 8]) > 0);
}

// set flag (value)
void ANVIL__set__flag_in_context(ANVIL__context* context, ANVIL__flag_ID flag_ID, ANVIL__bt value) {
    ANVIL__u8 masks[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

    // clear bit
    ((((ANVIL__u8*)&((*context).registers[ANVIL__rt__flags_0]))[flag_ID / 8] &= (~masks[flag_ID % 8])));

    // set bit
    ((ANVIL__u8*)&((*context).registers[ANVIL__rt__flags_0]))[flag_ID / 8] |= (value << (flag_ID % 8));

    return;
}

// read item from buffer and advance read address
ANVIL__u64 ANVIL__read_next__buffer_item(ANVIL__register* address, ANVIL__byte_count byte_count) {
    ANVIL__u64 output;

    // read data from buffer
    output = ANVIL__read__buffer(*address, byte_count);

    // advance pointer
    *address = (ANVIL__address)(*((u8**)address) + byte_count);

    // return data
    return output;
}

// get register value and advance
ANVIL__register ANVIL__read_next__register(ANVIL__register* address) {
    // read data
    return (ANVIL__register)ANVIL__read_next__buffer_item(address, sizeof(ANVIL__register));
}

// get instruction ID and advance
ANVIL__instruction_ID ANVIL__read_next__instruction_ID(ANVIL__register* address) {
    // read data
    return (ANVIL__instruction_ID)ANVIL__read_next__buffer_item(address, sizeof(ANVIL__instruction_ID));
}

// get flag ID and advance
ANVIL__flag_ID ANVIL__read_next__flag_ID(ANVIL__register* address) {
    // read data
    return (ANVIL__flag_ID)ANVIL__read_next__buffer_item(address, sizeof(ANVIL__flag_ID));
}

// get operation ID and advance
ANVIL__operation_ID ANVIL__read_next__operation_ID(ANVIL__register* address) {
    // read data
    return (ANVIL__operation_ID)ANVIL__read_next__buffer_item(address, sizeof(ANVIL__operation_ID));
}

// get register ID and advance
ANVIL__register_ID ANVIL__read_next__register_ID(ANVIL__register* address) {
    // read data
    return (ANVIL__register_ID)ANVIL__read_next__buffer_item(address, sizeof(ANVIL__register_ID));
}

/* Setup Alloy Code */
// create a skeleton context
ANVIL__context ANVIL__setup__context(ANVIL__buffer program) {
    ANVIL__context output;

    // setup program and execution registers
    output.registers[ANVIL__rt__program_start_address] = (ANVIL__register)program.start;
    output.registers[ANVIL__rt__program_end_address] = (ANVIL__register)program.end;
    output.registers[ANVIL__rt__program_current_address] = output.registers[ANVIL__rt__program_start_address];

    return output;
}

/* Run Alloy Code */
void ANVIL__run__context(ANVIL__context* context, ANVIL__instruction_count instruction_count);

// process operation (assumes flag was checked)
ANVIL__nit ANVIL__run__operation(ANVIL__context* context, ANVIL__ot operation_type, ANVIL__register_ID input_0, ANVIL__register_ID input_1, ANVIL__register_ID input_2, ANVIL__register_ID output_0) {
    ANVIL__register_integer_value temp_input_0;
    ANVIL__register_integer_value temp_input_1;
    ANVIL__register_integer_value temp_input_2;
    ANVIL__register_integer_value temp_result;

    // do operation based on type
    switch (operation_type) {
    // register to register
    case ANVIL__ot__register_to_register:
        // set value
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), ANVIL__get__register_from_context(context, input_0));

        break;
    // get register dynamically
    case ANVIL__ot__fetch_register: // untested!
        // set value
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)(ANVIL__u64)ANVIL__get__register_from_context(context, (ANVIL__register_ID)(ANVIL__u64)ANVIL__get__register_from_context(context, input_0)));

        break;
    // binary or
    case ANVIL__ot__bits_or:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 | temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary invert
    case ANVIL__ot__bits_invert:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);

        // perform operation
        temp_result = ~temp_input_0;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary and
    case ANVIL__ot__bits_and:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 & temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary xor
    case ANVIL__ot__bits_xor:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 ^ temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary bit shift higher
    case ANVIL__ot__bits_shift_higher:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 << temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary bit shift lower
    case ANVIL__ot__bits_shift_lower:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 >> temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // binary overwrite bits
    case ANVIL__ot__bits_overwrite:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // mask (positive bits are the ones being overwritten!)
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1); // old bits
        temp_input_2 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_2); // new bits

        // perform operation
        temp_result = (~temp_input_0) & temp_input_1;
        temp_result = temp_result | (temp_input_2 & temp_input_0);

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer addition
    case ANVIL__ot__integer_add:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 + temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer subtraction
    case ANVIL__ot__integer_subtract:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 - temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer multiplication
    case ANVIL__ot__integer_multiply:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // perform operation
        temp_result = temp_input_0 * temp_input_1;

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer division
    case ANVIL__ot__integer_division:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // check if division is possible
        if (temp_input_1 != 0) {
            // compute result
            temp_result = temp_input_0 / temp_input_1;
        } else {
            // set error
            ANVIL__set__error_code_register(context, ANVIL__et__divide_by_zero);

            // set blank output
            temp_result = 0;
        }

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer modulous
    case ANVIL__ot__integer_modulous:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0);
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1);

        // check if modulous is possible
        if (temp_input_1 != 0) {
            // compute result
            temp_result = temp_input_0 / temp_input_1;
        } else {
            // set error
            ANVIL__set__error_code_register(context, ANVIL__et__modulous_by_zero);

            // set blank output
            temp_result = 0;
        }

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // integer range check
    case ANVIL__ot__integer_within_range:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // range start
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1); // value to be checked
        temp_input_2 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_2); // range end

        // check range
        temp_result = (ANVIL__register_integer_value)((temp_input_0 <= temp_input_1) && (temp_input_1 <= temp_input_2));

        // set result register
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)temp_result);

        break;
    // flag or
    case ANVIL__ot__flag_or: // untested!
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // first flag
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1); // second flag
        temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // output flag

        // 'or' flags into new flag
        ANVIL__set__flag_in_context(context, temp_result, (ANVIL__bt)(ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_0) | ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_1)));

        break;
    // flag invert
    case ANVIL__ot__flag_invert: // untested!
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // first flag
        temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // output flag

        // 'or' flags into new flag
        ANVIL__set__flag_in_context(context, temp_result, (ANVIL__bt)!(ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_0)));

        break;
    // flag and
    case ANVIL__ot__flag_and: // untested!
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // first flag
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1); // second flag
        temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // output flag

        // 'or' flags into new flag
        ANVIL__set__flag_in_context(context, temp_result, (ANVIL__bt)(ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_0) & ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_1)));

        break;
    // flag xor
    case ANVIL__ot__flag_xor: // untested!
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // first flag
        temp_input_1 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_1); // second flag
        temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // output flag

        // 'xor' flags into new flag
        ANVIL__set__flag_in_context(context, temp_result, (ANVIL__bt)(ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_0) != ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_1)));

        break;
    // flag get
    case ANVIL__ot__flag_get:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // flag address
        //temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // output register

        // get flag
        ANVIL__set__register_by_address(ANVIL__get__register_address_from_context(context, output_0), (ANVIL__register)(ANVIL__u64)(ANVIL__get__flag_from_context(context, (ANVIL__flag_ID)temp_input_0) > 0));

        break;
    // flag set
    case ANVIL__ot__flag_set:
        // get data
        temp_input_0 = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, input_0); // flag value
        temp_result = (ANVIL__register_integer_value)ANVIL__get__register_from_context(context, output_0); // destination flag ID

        // set flag
        ANVIL__set__flag_in_context(context, (ANVIL__flag_ID)temp_result, (ANVIL__bt)temp_input_0);

        break;
    // instruction ID was invalid
    default:
        // return failed instruction
        return ANVIL__nit__return_context;
    }

    return ANVIL__nit__next_instruction;
}

// process instruction
ANVIL__nit ANVIL__run__instruction(ANVIL__context* context) {
    // execution current read address
    ANVIL__register* execution_read_address;

    // instruction ID
    ANVIL__it instruction_ID;

    // write register temps
    ANVIL__register write_register__register;
    ANVIL__register_ID write_register__register_ID;

    // operate temps
    ANVIL__flag_ID operate__flag_ID;
    ANVIL__operation_ID operate__operation_ID;
    ANVIL__register_ID operate__input_register_ID_0;
    ANVIL__register_ID operate__input_register_ID_1;
    ANVIL__register_ID operate__input_register_ID_2;
    ANVIL__register_ID operate__output_register_ID_0;

    // request memory temps
    ANVIL__register_ID request_memory__allocation_size;
    ANVIL__register_ID request_memory__allocation_start;
    ANVIL__register_ID request_memory__allocation_end;
    ANVIL__buffer request_memory__allocation;

    // return memory temps
    ANVIL__register_ID return_memory__allocation_start;
    ANVIL__register_ID return_memory__allocation_end;
    ANVIL__buffer return_memory__allocation;

    // address to register temps
    ANVIL__flag_ID address_to_register__flag_ID;
    ANVIL__length address_to_register__source_register_ID;
    ANVIL__length address_to_register__bit_count_register_ID;
    ANVIL__length address_to_register__destination_register_ID;

    // register to address temps
    ANVIL__flag_ID register_to_address__flag_ID;
    ANVIL__length register_to_address__source_register_ID;
    ANVIL__length register_to_address__bit_count_register_ID;
    ANVIL__length register_to_address__destination_register_ID;

    // file to buffer temps
    ANVIL__register_ID file_to_buffer__file_name_start;
    ANVIL__register_ID file_to_buffer__file_name_end;
    ANVIL__register_ID file_to_buffer__file_data_start;
    ANVIL__register_ID file_to_buffer__file_data_end;
    ANVIL__buffer file_to_buffer__file_name;
    ANVIL__buffer file_to_buffer__file_data;

    // buffer to file temps
    ANVIL__register_ID buffer_to_file__file_data_start;
    ANVIL__register_ID buffer_to_file__file_data_end;
    ANVIL__register_ID buffer_to_file__file_name_start;
    ANVIL__register_ID buffer_to_file__file_name_end;
    ANVIL__buffer buffer_to_file__file_data;
    ANVIL__buffer buffer_to_file__file_name;
    ANVIL__bt buffer_to_file__error;

    // debug putchar temps
    ANVIL__register_ID debug__putchar__printing_register_ID;

    // debug print register as decimal temps
    ANVIL__register_ID debug__print_register_as_decimal__printing_register_ID;

    // debug fgets temps
    ANVIL__register_ID debug__fgets__buffer_address_start;
    ANVIL__register_ID debug__fgets__buffer_address_end;
    ANVIL__u8 debug__fgets__temporary_string[ANVIL__define__input_string_max_length];
    ANVIL__buffer debug__fgets__buffer;
    ANVIL__length debug__fgets__buffer_length;

    // debug mark data section temps
    ANVIL__register debug__mark_data_section__section_length;

    // debug mark code section temps
    ANVIL__register debug__mark_code_section__section_length;

    // setup execution read address
    execution_read_address = ANVIL__get__register_address_from_context(context, ANVIL__rt__program_current_address);

    // get instruction ID from program
    instruction_ID = (ANVIL__it)ANVIL__read_next__instruction_ID(execution_read_address);

    // DEBUG
    // printf("[%lu]: instruction_ID: %lu\n", (ANVIL__u64)(*execution_read_address) - 1, (ANVIL__u64)instruction_ID);

    // process instruction accordingly
    switch (instruction_ID) {
    // if context should stop
    case ANVIL__it__stop:
        // DEBUG
        //printf("ANVIL__it__stop is running.\n");

        // return exit context
        return ANVIL__nit__return_context;
    // overwrite register value
    case ANVIL__it__write_register:
        // get parameters
        write_register__register = ANVIL__read_next__register(execution_read_address);
        write_register__register_ID = ANVIL__read_next__register_ID(execution_read_address);

        // do action
        (*context).registers[write_register__register_ID] = write_register__register;

        break;
    // operate between registers
    case ANVIL__it__operate:
        // get parameters
        operate__flag_ID = ANVIL__read_next__flag_ID(execution_read_address);
        operate__operation_ID = ANVIL__read_next__operation_ID(execution_read_address);
        operate__input_register_ID_0 = ANVIL__read_next__register_ID(execution_read_address);
        operate__input_register_ID_1 = ANVIL__read_next__register_ID(execution_read_address);
        operate__input_register_ID_2 = ANVIL__read_next__register_ID(execution_read_address);
        operate__output_register_ID_0 = ANVIL__read_next__register_ID(execution_read_address);

        // if flag enabled
        if (ANVIL__get__flag_from_context(context, operate__flag_ID) == ANVIL__bt__true) {
            // perform operation
            return ANVIL__run__operation(context, (ANVIL__ot)operate__operation_ID, operate__input_register_ID_0, operate__input_register_ID_1, operate__input_register_ID_2, operate__output_register_ID_0);
        }

        break;
    // ask os for new buffer
    case ANVIL__it__request_memory:
        // get parameters
        request_memory__allocation_size = ANVIL__read_next__register_ID(execution_read_address);
        request_memory__allocation_start = ANVIL__read_next__register_ID(execution_read_address);
        request_memory__allocation_end = ANVIL__read_next__register_ID(execution_read_address);

        // do action
        request_memory__allocation = ANVIL__open__buffer((ANVIL__length)(*context).registers[request_memory__allocation_size]);

        // set register data
        (*context).registers[request_memory__allocation_start] = request_memory__allocation.start;
        (*context).registers[request_memory__allocation_end] = request_memory__allocation.end;

        break;
    // return buffer to OS
    case ANVIL__it__return_memory:
        // get parameters
        return_memory__allocation_start = ANVIL__read_next__register_ID(execution_read_address);
        return_memory__allocation_end = ANVIL__read_next__register_ID(execution_read_address);

        // get parameters
        return_memory__allocation.start = (*context).registers[return_memory__allocation_start];
        return_memory__allocation.end = (*context).registers[return_memory__allocation_end];

        // deallocate
        ANVIL__close__buffer(return_memory__allocation);

        break;
    // take data from an address and put it into a register
    case ANVIL__it__address_to_register:
        // get parameters
        address_to_register__flag_ID = ANVIL__read_next__flag_ID(execution_read_address);
        address_to_register__source_register_ID = ANVIL__read_next__register_ID(execution_read_address);
        address_to_register__bit_count_register_ID = ANVIL__read_next__register_ID(execution_read_address);
        address_to_register__destination_register_ID = ANVIL__read_next__register_ID(execution_read_address);

        // if flag enabled
        if (ANVIL__get__flag_from_context(context, address_to_register__flag_ID) == ANVIL__bt__true) {
            // read data into register
            (*context).registers[address_to_register__destination_register_ID] = (ANVIL__register)ANVIL__read__buffer((ANVIL__address)(*context).registers[address_to_register__source_register_ID], ((ANVIL__length)(*context).registers[address_to_register__bit_count_register_ID]) / ANVIL__define__bits_in_byte);
        }

        break;
    // take data from a register and put it at an address
    case ANVIL__it__register_to_address:
        // get parameters
        register_to_address__flag_ID = ANVIL__read_next__flag_ID(execution_read_address);
        register_to_address__source_register_ID = ANVIL__read_next__register_ID(execution_read_address);
        register_to_address__bit_count_register_ID = ANVIL__read_next__register_ID(execution_read_address);
        register_to_address__destination_register_ID = ANVIL__read_next__register_ID(execution_read_address);

        // if flag enabled
        if (ANVIL__get__flag_from_context(context, register_to_address__flag_ID) == ANVIL__bt__true) {
            // write data to an address
            ANVIL__write__buffer((ANVIL__u64)(*context).registers[register_to_address__source_register_ID], ((ANVIL__length)(*context).registers[register_to_address__bit_count_register_ID]) / ANVIL__define__bits_in_byte, (*context).registers[register_to_address__destination_register_ID]);
        }

        break;
    // take data from a file and create a buffer with it
    case ANVIL__it__file_to_buffer:
        // get parameters
        file_to_buffer__file_name_start = ANVIL__read_next__register_ID(execution_read_address);
        file_to_buffer__file_name_end = ANVIL__read_next__register_ID(execution_read_address);
        file_to_buffer__file_data_start = ANVIL__read_next__register_ID(execution_read_address);
        file_to_buffer__file_data_end = ANVIL__read_next__register_ID(execution_read_address);

        // setup temps
        file_to_buffer__file_name.start = (*context).registers[file_to_buffer__file_name_start];
        file_to_buffer__file_name.end = (*context).registers[file_to_buffer__file_name_end];

        // get data from file
        file_to_buffer__file_data = ANVIL__move__file_to_buffer(file_to_buffer__file_name);

        // check for errors
        if (file_to_buffer__file_data.start == 0) {
            // set error
            ANVIL__set__error_code_register(context, ANVIL__et__file_not_found);
        }

        // write data to registers
        (*context).registers[file_to_buffer__file_data_start] = file_to_buffer__file_data.start;
        (*context).registers[file_to_buffer__file_data_end] = file_to_buffer__file_data.end;

        break;
    // take a buffer and overwrite a file with it
    case ANVIL__it__buffer_to_file:
        // get parameters
        buffer_to_file__file_data_start = ANVIL__read_next__register_ID(execution_read_address);
        buffer_to_file__file_data_end = ANVIL__read_next__register_ID(execution_read_address);
        buffer_to_file__file_name_start = ANVIL__read_next__register_ID(execution_read_address);
        buffer_to_file__file_name_end = ANVIL__read_next__register_ID(execution_read_address);

        // setup temps
        buffer_to_file__file_data.start = (*context).registers[buffer_to_file__file_data_start];
        buffer_to_file__file_data.end = (*context).registers[buffer_to_file__file_data_end];
        buffer_to_file__file_name.start = (*context).registers[buffer_to_file__file_name_start];
        buffer_to_file__file_name.end = (*context).registers[buffer_to_file__file_name_end];

        // create file
        ANVIL__move__buffer_to_file(&buffer_to_file__error, buffer_to_file__file_name, buffer_to_file__file_data);

        // check for errors
        if (buffer_to_file__error == ANVIL__bt__true) {
            // set error
            ANVIL__set__error_code_register(context, ANVIL__et__file_not_created);
        }

        break;
    // print one char to stdout
    case ANVIL__it__debug__putchar:
        // get parameters
        debug__putchar__printing_register_ID = ANVIL__read_next__register_ID(execution_read_address);

        // print
        putchar((u8)(u64)((*context).registers[debug__putchar__printing_register_ID]));

        // flush stream for full update
        fflush(stdout);

        break;
    // print one register as a decimal number
    case ANVIL__it__debug__print_register_as_decimal:
        // get parameters
        debug__print_register_as_decimal__printing_register_ID = ANVIL__read_next__register_ID(execution_read_address);

        // print
        printf("%lu", (u64)(*context).registers[debug__print_register_as_decimal__printing_register_ID]);

        break;
    // read one string from stdin
    case ANVIL__it__debug__fgets:
        // get parameters
        debug__fgets__buffer_address_start = ANVIL__read_next__register_ID(execution_read_address);
        debug__fgets__buffer_address_end = ANVIL__read_next__register_ID(execution_read_address);

        // zero out temporaries
        for (ANVIL__length i = 0; i < ANVIL__define__input_string_max_length; i++) {
            // zero out character
            debug__fgets__temporary_string[i] = 0;
        }
        debug__fgets__buffer_length = 0;

        // read string
        fgets((char*)debug__fgets__temporary_string, ANVIL__define__input_string_max_length, stdin);

        // read buffer for string size
        while (debug__fgets__buffer_length < ANVIL__define__input_string_max_length && debug__fgets__temporary_string[debug__fgets__buffer_length] != 0) {
            // increment length
            debug__fgets__buffer_length++;
        }

        // create buffer based on string length
        debug__fgets__buffer = ANVIL__open__buffer(debug__fgets__buffer_length);

        // copy data into buffer
        for (ANVIL__u64 i = 0; i < debug__fgets__buffer_length; i++) {
            // write character
            ANVIL__write__buffer((u8)debug__fgets__temporary_string[i], sizeof(ANVIL__u8), (ANVIL__u8*)debug__fgets__buffer.start + (i * sizeof(ANVIL__u8)));
        }

        // setup registers
        (*context).registers[debug__fgets__buffer_address_start] = debug__fgets__buffer.start;
        (*context).registers[debug__fgets__buffer_address_end] = debug__fgets__buffer.end;

        break;
    // mark section of data
    case ANVIL__it__debug__mark_data_section:
        // get parameters
        debug__mark_data_section__section_length = ANVIL__read_next__register(execution_read_address);

        // skip over data section
        (*context).registers[ANVIL__rt__program_current_address] = (ANVIL__address)((u64)(*context).registers[ANVIL__rt__program_current_address] + (u64)debug__mark_data_section__section_length);

        break;
    // mark section of code
    case ANVIL__it__debug__mark_code_section:
        // instruction does nothing but mark code space, so get length and do nothing
        debug__mark_code_section__section_length = ANVIL__read_next__register(execution_read_address);

        // useless operation to quiet compiler
        debug__mark_code_section__section_length = debug__mark_code_section__section_length;

        break;
    // in case instruction ID was invalid
    default:
        // set error
        ANVIL__set__error_code_register(context, ANVIL__et__invalid_instruction_ID);

        // return exit context
        return ANVIL__nit__return_context;
    }

    // return next instruction by default
    return ANVIL__nit__next_instruction;
}

// run context
void ANVIL__run__context(ANVIL__context* context, ANVIL__instruction_count instruction_count) {
    ANVIL__nit next_instruction_action;

    // if an infinite amount of instructions can execute
    if (instruction_count == ANVIL__define__run_forever) {
        // run instructions
        while (1) {
            // run instruction
            next_instruction_action = ANVIL__run__instruction(context);

            // if quit
            if (next_instruction_action == ANVIL__nit__return_context) {
                return;
            }

            // assuming ANVIL__nit__next_instruction
            continue;
        }
    // if a finite amount of instructions can execute
    } else {
        for (ANVIL__instruction_count i = 0; i < instruction_count; i++) {
            // run instruction
            next_instruction_action = ANVIL__run__instruction(context);

            // check for early quitting
            if (next_instruction_action == ANVIL__nit__return_context) {
                return;
            }
        }
    }

    return;
}

/* Instruction Workspace */
// pass type
typedef enum ANVIL__pt {
    ANVIL__pt__get_offsets,
    ANVIL__pt__write_program,
    ANVIL__pt__COUNT,
} ANVIL__pt;

// offsets
typedef ANVIL__u64 ANVIL__offset;

// invalid offset placeholder
#define ANVIL__invalid_offset -1

// instruction creation container
typedef struct ANVIL__workspace {
    ANVIL__pt pass;
    ANVIL__u64 current_program_offset;
    ANVIL__address write_to;
    ANVIL__buffer* program_buffer;
} ANVIL__workspace;

// setup workspace
ANVIL__workspace ANVIL__setup__workspace(ANVIL__buffer* program_buffer_destination) {
    ANVIL__workspace output;

    // setup output
    output.pass = ANVIL__pt__get_offsets;
    output.current_program_offset = 0;
    output.write_to = (*program_buffer_destination).start;
    output.program_buffer = program_buffer_destination;

    return output;
}

// create an offset
ANVIL__offset ANVIL__get__offset(ANVIL__workspace* workspace) {
    // return current offset
    return (*workspace).current_program_offset;
}

// setup pass
void ANVIL__setup__pass(ANVIL__workspace* workspace, ANVIL__pt pass) {
    // setup pass in workspace
    (*workspace).pass = pass;

    // do stuff
    switch ((*workspace).pass) {
    case ANVIL__pt__get_offsets:
        (*workspace).current_program_offset = 0;
        
        break;
    case ANVIL__pt__write_program:
        // allocate program buffer
        (*(*workspace).program_buffer) = ANVIL__open__buffer((*workspace).current_program_offset);

        // setup pass
        (*workspace).current_program_offset = 0;
        (*workspace).write_to = (*(*workspace).program_buffer).start;

        break;
    default:
        break;
    }

    return;
}

/* Write Instruction Scraplets */
// write instruction ID
void ANVIL__write_next__instruction_ID(ANVIL__workspace* workspace, ANVIL__instruction_ID instruction_ID) {
    // write value
    if ((*workspace).pass == ANVIL__pt__write_program) {
        *((ANVIL__instruction_ID*)(*workspace).write_to) = instruction_ID;
    }

    // advance
    (*workspace).current_program_offset += sizeof(ANVIL__instruction_ID);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + sizeof(ANVIL__instruction_ID));

    return;
}

// write flag ID
void ANVIL__write_next__flag_ID(ANVIL__workspace* workspace, ANVIL__flag_ID flag_ID) {
    // write value
    if ((*workspace).pass == ANVIL__pt__write_program) {
        *((ANVIL__flag_ID*)(*workspace).write_to) = flag_ID;
    }

    // advance
    (*workspace).current_program_offset += sizeof(ANVIL__flag_ID);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + sizeof(ANVIL__flag_ID));

    return;
}

// write operation ID
void ANVIL__write_next__operation_ID(ANVIL__workspace* workspace, ANVIL__operation_ID operation_ID) {
    // write value
    if ((*workspace).pass == ANVIL__pt__write_program) {
        *((ANVIL__operation_ID*)(*workspace).write_to) = operation_ID;
    }

    // advance
    (*workspace).current_program_offset += sizeof(ANVIL__operation_ID);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + sizeof(ANVIL__operation_ID));

    return;
}

// write register ID
void ANVIL__write_next__register_ID(ANVIL__workspace* workspace, ANVIL__register_ID register_ID) {
    // write value
    if ((*workspace).pass == ANVIL__pt__write_program) {
        *((ANVIL__register_ID*)(*workspace).write_to) = register_ID;
    }

    // advance
    (*workspace).current_program_offset += sizeof(ANVIL__register_ID);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + sizeof(ANVIL__register_ID));

    return;
}

// write register value
void ANVIL__write_next__register(ANVIL__workspace* workspace, ANVIL__register register_value) {
    // write value
    if ((*workspace).pass == ANVIL__pt__write_program) {
        *((ANVIL__register*)(*workspace).write_to) = register_value;
    }

    // advance
    (*workspace).current_program_offset += sizeof(ANVIL__register);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + sizeof(ANVIL__register));

    return;
}

// write buffer
void ANVIL__write_next__buffer(ANVIL__workspace* workspace, ANVIL__buffer buffer) {
    ANVIL__length buffer_length;

    // set buffer length
    buffer_length = ANVIL__calculate__buffer_length(buffer);

    // DEBUG
    printf("Buffer length: %lu\n", buffer_length);

    // write buffer
    if ((*workspace).pass == ANVIL__pt__write_program) {
        // write buffer length
        ANVIL__write__buffer(buffer_length, sizeof(ANVIL__length), (*workspace).write_to);

        // copy buffer
        for (ANVIL__length byte_index = 0; byte_index < buffer_length; byte_index++) {
            ((ANVIL__u8*)(*workspace).write_to)[byte_index + sizeof(ANVIL__length)] = ((ANVIL__u8*)buffer.start)[byte_index];
        }
    }

    // advance
    (*workspace).current_program_offset += buffer_length + sizeof(ANVIL__length);
    (*workspace).write_to = (ANVIL__address)((u64)(*workspace).write_to + buffer_length + sizeof(ANVIL__length));

    return;
}

/* Write Instructions */
// write buffer data
void ANVIL__code__buffer(ANVIL__workspace* workspace, ANVIL__buffer buffer) {
    // write data
    ANVIL__write_next__buffer(workspace, buffer);

    return;
}

// write stop instruction
void ANVIL__code__stop(ANVIL__workspace* workspace) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__stop);

    return;
}

// write write register instruction
void ANVIL__code__write_register(ANVIL__workspace* workspace, ANVIL__register value, ANVIL__register_ID value_destination) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__write_register);
    ANVIL__write_next__register(workspace, value);
    ANVIL__write_next__register_ID(workspace, value_destination);

    return;
}

// write operate instruction
void ANVIL__code__operate(ANVIL__workspace* workspace, ANVIL__flag_ID flag_ID, ANVIL__operation_ID operation_ID, ANVIL__register_ID input_0, ANVIL__register_ID input_1, ANVIL__register_ID input_2, ANVIL__register_ID output_0) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__operate);
    ANVIL__write_next__flag_ID(workspace, flag_ID);
    ANVIL__write_next__operation_ID(workspace, operation_ID);
    ANVIL__write_next__register_ID(workspace, input_0);
    ANVIL__write_next__register_ID(workspace, input_1);
    ANVIL__write_next__register_ID(workspace, input_2);
    ANVIL__write_next__register_ID(workspace, output_0);

    return;
}

// write request memory instruction
void ANVIL__code__request_memory(ANVIL__workspace* workspace, ANVIL__register_ID allocation_size, ANVIL__register_ID allocation_start, ANVIL__register_ID allocation_end) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__request_memory);
    ANVIL__write_next__register_ID(workspace, allocation_size);
    ANVIL__write_next__register_ID(workspace, allocation_start);
    ANVIL__write_next__register_ID(workspace, allocation_end);

    return;
}

// write return memory instruction
void ANVIL__code__return_memory(ANVIL__workspace* workspace, ANVIL__register_ID allocation_start, ANVIL__register_ID allocation_end) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__return_memory);
    ANVIL__write_next__register_ID(workspace, allocation_start);
    ANVIL__write_next__register_ID(workspace, allocation_end);

    return;
}

// write address to register instruction
void ANVIL__code__address_to_register(ANVIL__workspace* workspace, ANVIL__flag_ID flag_ID, ANVIL__register_ID source_address, ANVIL__register_ID bit_count, ANVIL__register_ID destination_register) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__address_to_register);
    ANVIL__write_next__flag_ID(workspace, flag_ID);
    ANVIL__write_next__register_ID(workspace, source_address);
    ANVIL__write_next__register_ID(workspace, bit_count);
    ANVIL__write_next__register_ID(workspace, destination_register);

    return;
}

// write register to address instruction
void ANVIL__code__register_to_address(ANVIL__workspace* workspace, ANVIL__flag_ID flag_ID, ANVIL__register_ID source_register, ANVIL__register_ID bit_count, ANVIL__register_ID destination_address) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__register_to_address);
    ANVIL__write_next__flag_ID(workspace, flag_ID);
    ANVIL__write_next__register_ID(workspace, source_register);
    ANVIL__write_next__register_ID(workspace, bit_count);
    ANVIL__write_next__register_ID(workspace, destination_address);

    return;
}

// write file to buffer instruction
void ANVIL__code__file_to_buffer(ANVIL__workspace* workspace, ANVIL__register_ID file_name_start, ANVIL__register_ID file_name_end, ANVIL__register_ID file_data_start, ANVIL__register_ID file_data_end) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__file_to_buffer);
    ANVIL__write_next__register_ID(workspace, file_name_start);
    ANVIL__write_next__register_ID(workspace, file_name_end);
    ANVIL__write_next__register_ID(workspace, file_data_start);
    ANVIL__write_next__register_ID(workspace, file_data_end);

    return;
}

// write buffer to file instruction
void ANVIL__code__buffer_to_file(ANVIL__workspace* workspace, ANVIL__register_ID file_data_start, ANVIL__register_ID file_data_end, ANVIL__register_ID file_name_start, ANVIL__register_ID file_name_end) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__buffer_to_file);
    ANVIL__write_next__register_ID(workspace, file_data_start);
    ANVIL__write_next__register_ID(workspace, file_data_end);
    ANVIL__write_next__register_ID(workspace, file_name_start);
    ANVIL__write_next__register_ID(workspace, file_name_end);

    return;
}

// write debug putchar instruction
void ANVIL__code__debug__putchar(ANVIL__workspace* workspace, ANVIL__register_ID printing_register_ID) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__debug__putchar);
    ANVIL__write_next__register_ID(workspace, printing_register_ID);

    return;
}

// write debug print register as decimal instruction
void ANVIL__code__debug__print_register_as_decimal(ANVIL__workspace* workspace, ANVIL__register_ID printing_register_ID) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__debug__print_register_as_decimal);
    ANVIL__write_next__register_ID(workspace, printing_register_ID);

    return;
}

// write debug fgets instruction
void ANVIL__code__debug__fgets(ANVIL__workspace* workspace, ANVIL__register_ID buffer_start_ID, ANVIL__register_ID buffer_end_ID) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__debug__fgets);
    ANVIL__write_next__register_ID(workspace, buffer_start_ID);
    ANVIL__write_next__register_ID(workspace, buffer_end_ID);

    return;
}

// write debug mark data section instruction
void ANVIL__code__debug__mark_data_section(ANVIL__workspace* workspace, ANVIL__register buffer_length) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__debug__mark_data_section);
    ANVIL__write_next__register(workspace, buffer_length);

    return;
}

// write debug mark code section instruction
void ANVIL__code__debug__mark_code_section(ANVIL__workspace* workspace, ANVIL__register code_buffer_length) {
    // write instruction
    ANVIL__write_next__instruction_ID(workspace, ANVIL__it__debug__mark_code_section);
    ANVIL__write_next__register(workspace, code_buffer_length);

    return;
}

/* Stack ABI Defines */
// types
typedef u64 ANVIL__preserve;
typedef ANVIL__preserve ANVIL__preserve__start;
typedef ANVIL__preserve ANVIL__preserve__end;

// stack register types
typedef enum ANVIL__srt {
    // start of registers
    ANVIL__srt__START = ANVIL__rt__END,

    // constant registers
    ANVIL__srt__constant__0 = ANVIL__srt__START,
    ANVIL__srt__constant__1,
    ANVIL__srt__constant__2,
    ANVIL__srt__constant__4,
    ANVIL__srt__constant__8,
    ANVIL__srt__constant__16,
    ANVIL__srt__constant__24,
    ANVIL__srt__constant__32,
    ANVIL__srt__constant__40,
    ANVIL__srt__constant__48,
    ANVIL__srt__constant__56,
    ANVIL__srt__constant__64,
    ANVIL__srt__constant__register_byte_size,
    ANVIL__srt__constant__return_address_offset_creation_size,

    // context io registers
    ANVIL__srt__input_buffer_start,
    ANVIL__srt__input_buffer_end,
    ANVIL__srt__output_buffer_start,
    ANVIL__srt__output_buffer_end,

    // stack registers
    ANVIL__srt__stack__start_address,
    ANVIL__srt__stack__current_address,
    ANVIL__srt__stack__end_address,

    // control flow registers
    ANVIL__srt__return_address,

    // temporary registers
    ANVIL__srt__temp__write,
    ANVIL__srt__temp__offset,
    ANVIL__srt__temp__address,
    ANVIL__srt__temp__flag,
    ANVIL__srt__temp__flag_ID,
    ANVIL__srt__temp__bit_count,

    // end of registers
    ANVIL__srt__END,

    // aliases
    ANVIL__srt__constant__true = ANVIL__srt__constant__1,
    ANVIL__srt__constant__false = ANVIL__srt__constant__0,
    ANIVL__srt__constant__register_byte_count = ANVIL__srt__constant__8,
    ANIVL__srt__constant__register_bit_count = ANVIL__srt__constant__64,
    ANVIL__srt__constant__bits_in_byte = ANVIL__srt__constant__8,

    // locations
    ANVIL__srt__start__workspace = 64,
    ANVIL__srt__start__function_io = 224,

    // count
    ANVIL__srt__COUNT = ANVIL__srt__END - ANVIL__srt__START,
} ANVIL__srt;

// stack instruction length types
typedef enum ANVIL__silt {
    ANVIL__silt__register_to_register = ANVIL__ilt__operate,
    ANVIL__silt__push_register = ANVIL__ilt__register_to_address + ANVIL__ilt__operate,
    ANVIL__silt__pop_register = ANVIL__ilt__operate + ANVIL__ilt__address_to_register,
    ANVIL__silt__calculate_dynamically__offset_address = ANVIL__ilt__operate,
    ANVIL__silt__calculate_statically__offset_address = ANVIL__ilt__write_register + ANVIL__silt__calculate_dynamically__offset_address,
    ANVIL__silt__jump__explicit = ANVIL__silt__register_to_register,
    ANVIL__silt__jump__static = ANVIL__silt__calculate_statically__offset_address + ANVIL__silt__jump__explicit,
} ANVIL__silt;

// stack flag types
typedef enum ANVIL__sft {
    // start of flags
    ANVIL__sft__START = 0,

    // flags
    ANVIL__sft__always_run = ANVIL__sft__START,
    ANVIL__sft__never_run,
    ANVIL__sft__temp,

    // end of stack flags
    ANVIL__sft__END,
} ANVIL__sft;

// stack size
typedef u64 ANVIL__stack_size;

/* Context IO */
// pass input
void ANVIL__set__input(ANVIL__context* context, ANVIL__buffer input) {
    // write data to registers
    (*context).registers[ANVIL__srt__input_buffer_start] = (ANVIL__register)input.start;
    (*context).registers[ANVIL__srt__input_buffer_end] = (ANVIL__register)input.end;

    return;
}

/* Stack ABI Code */
// move one register to the next
void ANVIL__code__register_to_register(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID source, ANVIL__register_ID destination) {
    // write code
    ANVIL__code__operate(workspace, flag, ANVIL__ot__register_to_register, source, ANVIL__unused_register_ID, ANVIL__unused_register_ID, destination);

    return;
}

// push a register onto the stack
void ANVIL__code__push_register(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID source_register) {
    // write data to stack
    ANVIL__code__register_to_address(workspace, flag, source_register, ANVIL__srt__constant__64, ANVIL__srt__stack__current_address);

    // increase stack pointer
    ANVIL__code__operate(workspace, flag, ANVIL__ot__integer_add, ANVIL__srt__stack__current_address, ANVIL__srt__constant__register_byte_size, ANVIL__unused_register_ID, ANVIL__srt__stack__current_address);

    return;
}

// pop a register from the stack
void ANVIL__code__pop_register(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID destination_register) {
    // decrease stack pointer
    ANVIL__code__operate(workspace, flag, ANVIL__ot__integer_subtract, ANVIL__srt__stack__current_address, ANVIL__srt__constant__register_byte_size, ANVIL__unused_register_ID, ANVIL__srt__stack__current_address);

    // read data from stack
    ANVIL__code__address_to_register(workspace, flag, ANVIL__srt__stack__current_address, ANVIL__srt__constant__64, destination_register);

    return;
}

// calculate an address from the program start and an offset register
void ANVIL__code__calculate_dynamically__offset_address(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID offset_register, ANVIL__register_ID destination) {
    // calculate address
    ANVIL__code__operate(workspace, flag, ANVIL__ot__integer_add, ANVIL__rt__program_start_address, offset_register, ANVIL__unused_register_ID, destination);

    return;
}

// calculate an address from the program start and an offset
void ANVIL__code__calculate_statically__offset_address(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__offset offset, ANVIL__register_ID destination) {
    // write temp
    ANVIL__code__write_register(workspace, (ANVIL__register)offset, ANVIL__srt__temp__offset);

    // calculate address
    ANVIL__code__calculate_dynamically__offset_address(workspace, flag, ANVIL__srt__temp__offset, destination);

    return;
}

// jump to a specific address
void ANVIL__code__jump__explicit(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID address) {
    // jump
    ANVIL__code__register_to_register(workspace, flag, address, ANVIL__rt__program_current_address);

    return;
}

// jump to an offset calculated address
void ANVIL__code__jump__static(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__offset offset) {
    // calculate offset
    ANVIL__code__calculate_statically__offset_address(workspace, flag, offset, ANVIL__srt__temp__address);

    // jump
    ANVIL__code__jump__explicit(workspace, flag, ANVIL__srt__temp__address);

    return;
}

// create return address
void ANVIL__code__create_return_address__directly_after_jump(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID destination) {
    // create offset
    ANVIL__code__operate(workspace, flag, ANVIL__ot__integer_add, ANVIL__rt__program_current_address, ANVIL__srt__constant__return_address_offset_creation_size, ANVIL__unused_register_ID, destination);

    return;
}

// call function explicitly
void ANVIL__code__call__explicit(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID address) {
    // preserve return address
    ANVIL__code__push_register(workspace, flag, ANVIL__srt__return_address);

    // setup new return address
    ANVIL__code__create_return_address__directly_after_jump(workspace, flag, ANVIL__srt__return_address);

    // jump
    ANVIL__code__jump__explicit(workspace, flag, address);

    // restore return address
    ANVIL__code__pop_register(workspace, flag, ANVIL__srt__return_address);

    return;
}

// call function statically
void ANVIL__code__call__static(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__offset offset) {
    // calculate address
    ANVIL__code__calculate_statically__offset_address(workspace, flag, offset, ANVIL__srt__temp__address);

    // call function with offset set
    ANVIL__code__call__explicit(workspace, flag, ANVIL__srt__temp__address);

    return;
}

// kickstart program (assumes program variables are set!)
void ANVIL__code__start(ANVIL__workspace* workspace, ANVIL__stack_size stack_size, ANVIL__offset jump_to) {
    // setup code marker
    ANVIL__code__debug__mark_code_section(workspace, 0);

    // setup error register
    ANVIL__code__write_register(workspace, (ANVIL__register)0, ANVIL__rt__error_code);

    // setup flag registers
    ANVIL__code__write_register(workspace, (ANVIL__register)1, ANVIL__rt__flags_0);
    ANVIL__code__write_register(workspace, (ANVIL__register)0, ANVIL__rt__flags_1);
    ANVIL__code__write_register(workspace, (ANVIL__register)0, ANVIL__rt__flags_2);
    ANVIL__code__write_register(workspace, (ANVIL__register)0, ANVIL__rt__flags_3);

    // setup constants
    ANVIL__code__write_register(workspace, (ANVIL__register)0, ANVIL__srt__constant__0);
    ANVIL__code__write_register(workspace, (ANVIL__register)1, ANVIL__srt__constant__1);
    ANVIL__code__write_register(workspace, (ANVIL__register)2, ANVIL__srt__constant__2);
    ANVIL__code__write_register(workspace, (ANVIL__register)4, ANVIL__srt__constant__4);
    ANVIL__code__write_register(workspace, (ANVIL__register)8, ANVIL__srt__constant__8);
    ANVIL__code__write_register(workspace, (ANVIL__register)16, ANVIL__srt__constant__16);
    ANVIL__code__write_register(workspace, (ANVIL__register)24, ANVIL__srt__constant__24);
    ANVIL__code__write_register(workspace, (ANVIL__register)32, ANVIL__srt__constant__32);
    ANVIL__code__write_register(workspace, (ANVIL__register)40, ANVIL__srt__constant__40);
    ANVIL__code__write_register(workspace, (ANVIL__register)48, ANVIL__srt__constant__48);
    ANVIL__code__write_register(workspace, (ANVIL__register)56, ANVIL__srt__constant__56);
    ANVIL__code__write_register(workspace, (ANVIL__register)64, ANVIL__srt__constant__64);
    ANVIL__code__write_register(workspace, (ANVIL__register)sizeof(ANVIL__register), ANVIL__srt__constant__register_byte_size);
    ANVIL__code__write_register(workspace, (ANVIL__register)ANVIL__silt__jump__explicit, ANVIL__srt__constant__return_address_offset_creation_size);

    // setup output
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ANVIL__srt__constant__0, ANVIL__srt__output_buffer_start);
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ANVIL__srt__constant__0, ANVIL__srt__output_buffer_end);

    // setup stack
    ANVIL__code__write_register(workspace, (ANVIL__register)stack_size, ANVIL__srt__temp__write);
    ANVIL__code__request_memory(workspace, ANVIL__srt__temp__write, ANVIL__srt__stack__start_address, ANVIL__srt__stack__end_address);
    ANVIL__code__register_to_register(workspace, (ANVIL__flag_ID)ANVIL__sft__always_run, ANVIL__srt__stack__start_address, ANVIL__srt__stack__current_address);

    // jump to main
    ANVIL__code__call__static(workspace, ANVIL__sft__always_run, jump_to);

    // deallocate stack
    ANVIL__code__return_memory(workspace, ANVIL__srt__stack__start_address, ANVIL__srt__stack__end_address);

    // quit program
    ANVIL__code__stop(workspace);

    return;
}

// preserve workspace
void ANVIL__code__preserve_workspace(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__preserve__start preserve_start, ANVIL__preserve__end preserve_end) {
    // preserve flags
    ANVIL__code__push_register(workspace, flag, ANVIL__rt__flags_0);
    ANVIL__code__push_register(workspace, flag, ANVIL__rt__flags_1);
    ANVIL__code__push_register(workspace, flag, ANVIL__rt__flags_2);
    ANVIL__code__push_register(workspace, flag, ANVIL__rt__flags_3);

    // preserve error code
    ANVIL__code__push_register(workspace, flag, ANVIL__rt__error_code);

    // preserve workspace registers
    for (ANVIL__preserve i = preserve_start; i <= preserve_end; i++) {
        // preserve register
        ANVIL__code__push_register(workspace, flag, i);
    }

    return;
}

// restore workspace
void ANVIL__code__restore_workspace(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__preserve__start preserve_start, ANVIL__preserve__end preserve_end) {
    // preserve workspace registers
    for (ANVIL__preserve i = preserve_end; i >= preserve_start; i--) {
        // preserve register
        ANVIL__code__pop_register(workspace, flag, i);
    }

    // preserve error code
    ANVIL__code__pop_register(workspace, flag, ANVIL__rt__error_code);

    // restore flags
    ANVIL__code__pop_register(workspace, flag, ANVIL__rt__flags_3);
    ANVIL__code__pop_register(workspace, flag, ANVIL__rt__flags_2);
    ANVIL__code__pop_register(workspace, flag, ANVIL__rt__flags_1);
    ANVIL__code__pop_register(workspace, flag, ANVIL__rt__flags_0);

    return;
}

// operate flag
void ANVIL__code__operate__flag(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__register_ID output_flag_ID) {
    // get comparison result set to variable
    ANVIL__code__operate(workspace, flag, ANVIL__ot__integer_within_range, lower_bound, value, upper_bound, ANVIL__srt__temp__flag);

    // write flag
    ANVIL__code__operate(workspace, flag, ANVIL__ot__flag_set, ANVIL__srt__temp__flag, ANVIL__unused_register_ID, ANVIL__unused_register_ID, output_flag_ID);

    // invert flag
    ANVIL__code__operate(workspace, invert_result, ANVIL__ot__flag_invert, output_flag_ID, ANVIL__unused_register_ID, ANVIL__unused_register_ID, output_flag_ID);

    return;
}

// operate jump explicitly
void ANVIL__code__operate__jump__explicit(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__register_ID jump_address) {
    // setup flag temp
    ANVIL__code__write_register(workspace, (ANVIL__register)ANVIL__sft__temp, ANVIL__srt__temp__flag_ID);

    // perform comparison
    ANVIL__code__operate__flag(workspace, flag, lower_bound, value, upper_bound, invert_result, ANVIL__srt__temp__flag_ID);

    // attempt jump
    ANVIL__code__jump__explicit(workspace, ANVIL__sft__temp, jump_address);

    return;
}

// operate jump dynamically
void ANVIL__code__operate__jump__dynamic(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__register_ID jump_offset_register) {
    // calculate offset
    ANVIL__code__calculate_dynamically__offset_address(workspace, flag, jump_offset_register, ANVIL__srt__temp__address);

    // attempt jump
    ANVIL__code__operate__jump__explicit(workspace, flag, lower_bound, value, upper_bound, invert_result, ANVIL__srt__temp__address);

    return;
}

// operate jump statically
void ANVIL__code__operate__jump__static(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__offset jump_offset) {
    // write offset to register
    ANVIL__code__write_register(workspace, (ANVIL__register)jump_offset, ANVIL__srt__temp__offset);

    // attempt jump
    ANVIL__code__operate__jump__dynamic(workspace, flag, lower_bound, value, upper_bound, invert_result, ANVIL__srt__temp__offset);

    return;
}

/*// operate call explicitly
void ANVIL__code__operate__call__explicit(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__register_ID address) {
    // setup flag temp
    ANVIL__code__write_register(workspace, (ANVIL__register)ANVIL__sft__temp, ANVIL__srt__temp__flag_ID);

    // perform comparison
    ANVIL__code__operate__flag(workspace, flag, lower_bound, value, upper_bound, invert_result, ANVIL__srt__temp__flag_ID);

    // perform call
    ANVIL__code__call__explicit(workspace, ANVIL__sft__temp, address);

    return;
}

// operate call statically
void ANVIL__code__operate__call__static(ANVIL__workspace* workspace, ANVIL__flag_ID flag, ANVIL__register_ID lower_bound, ANVIL__register_ID value, ANVIL__register_ID upper_bound, ANVIL__flag_ID invert_result, ANVIL__offset jump_offset) {
    // setup flag temp
    ANVIL__code__write_register(workspace, (ANVIL__register)ANVIL__sft__temp, ANVIL__srt__temp__flag_ID);

    // perform comparison
    ANVIL__code__operate__flag(workspace, flag, lower_bound, value, upper_bound, invert_result, ANVIL__srt__temp__flag_ID);

    // perform call
    ANVIL__code__call__static(workspace, ANVIL__sft__temp, jump_offset);

    return;
}*/

// setup context
void ANVIL__code__setup__context(ANVIL__workspace* workspace, ANVIL__register_ID program_buffer_start, ANVIL__register_ID program_buffer_end, ANVIL__register_ID context_buffer_start, ANVIL__register_ID context_buffer_end) {
    // setup allocation size
    ANVIL__code__write_register(workspace, (ANVIL__register)sizeof(ANVIL__context), ANVIL__srt__temp__write);

    // allocate context
    ANVIL__code__request_memory(workspace, ANVIL__srt__temp__write, context_buffer_start, context_buffer_end);

    // setup skeleton context
    // setup buffer start
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, context_buffer_start, ANVIL__srt__temp__address);
    ANVIL__code__register_to_address(workspace, ANVIL__sft__always_run, program_buffer_start, ANIVL__srt__constant__register_bit_count, ANVIL__srt__temp__address);

    // setup current address
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_add, ANVIL__srt__temp__address, ANIVL__srt__constant__register_byte_count, ANVIL__unused_register_ID, ANVIL__srt__temp__address);
    ANVIL__code__register_to_address(workspace, ANVIL__sft__always_run, program_buffer_start, ANIVL__srt__constant__register_bit_count, ANVIL__srt__temp__address);

    // setup end address
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_add, ANVIL__srt__temp__address, ANIVL__srt__constant__register_byte_count, ANVIL__unused_register_ID, ANVIL__srt__temp__address);
    ANVIL__code__register_to_address(workspace, ANVIL__sft__always_run, program_buffer_end, ANIVL__srt__constant__register_bit_count, ANVIL__srt__temp__address);
    
    return;
}

#endif
