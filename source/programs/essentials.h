#ifndef ANVIL__essentials
#define ANVIL__essentials

/* Include */
// anvil
#include "../anvil/anvil.h"

/* Offsets */
// offset type
typedef enum ESS__ot {
    // print buffer
    ESS__ot__print_buffer__start,
    ESS__ot__print_buffer__loop__start,
    ESS__ot__print_buffer__return,

    // retrieve buffer
    ESS__ot__retrieve_buffer__start,

    // write byte
    ESS__ot__write_byte__start,
    ESS__ot__write_byte__return,

    // print register as decimal
    ESS__ot__print_hexadecimal__start,
    ESS__ot__print_hexadecimal__loop__start,
    ESS__ot__print_hexadecimal__return,

    // count
    ESS__ot__COUNT,
} ESS__ot;

// offsets
typedef struct ESS__offsets {
    ANVIL__offset offsets[ESS__ot__COUNT];
} ESS__offsets;

/* Printing */
// print buffer registers
typedef enum ESS__rt__print_buffer {
    // preserve start
    ESS__rt__print_buffer__preserve__START = ANVIL__srt__start__workspace,

    // variables
    ESS__rt__print_buffer__buffer_start = ESS__rt__print_buffer__preserve__START,
    ESS__rt__print_buffer__buffer_current,
    ESS__rt__print_buffer__buffer_end,
    ESS__rt__print_buffer__character,

    // preserve end
    ESS__rt__print_buffer__preserve__END,

    // inputs
    ESS__rt__print_buffer__input__buffer_start = ANVIL__srt__start__function_io,
    ESS__rt__print_buffer__input__buffer_end,

    // statistics
    ESS__rt__print_buffer__variable__COUNT = ESS__rt__print_buffer__preserve__END - ESS__rt__print_buffer__preserve__START,
} ESS__rt__print_buffer;

// call print buffer
void ESS__code__call__print_buffer(ANVIL__workspace* workspace, ESS__offsets* essential_offsets, ANVIL__flag_ID flag, ANVIL__register_ID buffer_start, ANVIL__register_ID buffer_end) {
    // pass parameters
    ANVIL__code__register_to_register(workspace, flag, buffer_start, ESS__rt__print_buffer__input__buffer_start);
    ANVIL__code__register_to_register(workspace, flag, buffer_end, ESS__rt__print_buffer__input__buffer_end);

    // call code
    ANVIL__code__call__static(workspace, flag, (*essential_offsets).offsets[ESS__ot__print_buffer__start]);

    return;
}

// print buffer
void ESS__code__print_buffer(ANVIL__workspace* workspace, ESS__offsets* essential_offsets) {
    // setup function start
    (*essential_offsets).offsets[ESS__ot__print_buffer__start] = ANVIL__get__offset(workspace);

    // setup function prologue
    ANVIL__code__preserve_workspace(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__preserve__START, ESS__rt__print_buffer__preserve__END);

    // get parameters
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__input__buffer_start, ESS__rt__print_buffer__buffer_start);
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__input__buffer_end, ESS__rt__print_buffer__buffer_end);

    // setup variables
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__buffer_start, ESS__rt__print_buffer__buffer_current);

    // setup loop offset
    (*essential_offsets).offsets[ESS__ot__print_buffer__loop__start] = ANVIL__get__offset(workspace);

    // check if there are any characters left to print
    ANVIL__code__operate__jump__static(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__buffer_start, ESS__rt__print_buffer__buffer_current, ESS__rt__print_buffer__buffer_end, ANVIL__sft__always_run, (*essential_offsets).offsets[ESS__ot__print_buffer__return]);

    // get character
    ANVIL__code__address_to_register(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__buffer_current, ANVIL__srt__constant__8, ESS__rt__print_buffer__character);

    // advance to next character
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_add, ESS__rt__print_buffer__buffer_current, ANVIL__srt__constant__1, ANVIL__unused_register_ID, ESS__rt__print_buffer__buffer_current);

    // print character
    ANVIL__code__debug__putchar(workspace, ESS__rt__print_buffer__character);

    // jump to beginning
    ANVIL__code__jump__static(workspace, ANVIL__sft__always_run, (*essential_offsets).offsets[ESS__ot__print_buffer__loop__start]);

    // setup return offset
    (*essential_offsets).offsets[ESS__ot__print_buffer__return] = ANVIL__get__offset(workspace);

    // setup function epilogue
    ANVIL__code__restore_workspace(workspace, ANVIL__sft__always_run, ESS__rt__print_buffer__preserve__START, ESS__rt__print_buffer__preserve__END);

    // jump to return address
    ANVIL__code__jump__explicit(workspace, ANVIL__sft__always_run, ANVIL__srt__return_address);

    return;
}

/* Quick Print Buffer */
void ESS__code__quick_print_buffer(ANVIL__workspace* workspace, ANVIL__buffer buffer) {
    ANVIL__address current;

    // setup loop
    current = buffer.start;

    // print each character individually
    while (current <= buffer.end) {
        // print character
        ANVIL__code__write_register(workspace, (ANVIL__register)(u64)(*((u8*)current)), ANVIL__srt__temp__write);
        ANVIL__code__debug__putchar(workspace, ANVIL__srt__temp__write);

        // next character
        current += sizeof(u8);
    }

    return;
}

/* Inline Buffer Retrieval */
typedef enum ESS__rt__retrieve_buffer {
    // preserve start
    ESS__rt__retrieve_buffer__preserve__START = ANVIL__srt__start__workspace,

    // variables
    ESS__rt__retrieve_buffer__bit_size_of_buffer_length = ESS__rt__retrieve_buffer__preserve__START,
    ESS__rt__retrieve_buffer__byte_size_of_buffer_length,
    ESS__rt__retrieve_buffer__buffer_length,
    ESS__rt__retrieve_buffer__message_buffer_start,
    ESS__rt__retrieve_buffer__message_data_start,
    ESS__rt__retrieve_buffer__message_buffer_end,

    // preserve end
    ESS__rt__retrieve_buffer__preserve__END,

    // inputs
    ESS__rt__retrieve_buffer__input__message_buffer_start_address = ANVIL__srt__start__function_io,

    // outputs
    ESS__rt__retrieve_buffer__output__buffer_start = ANVIL__srt__start__function_io,
    ESS__rt__retrieve_buffer__output__buffer_end,

    // statistics
    ESS__rt__retrieve_buffer__COUNT = ESS__rt__retrieve_buffer__preserve__END - ESS__rt__retrieve_buffer__preserve__START,
} ESS__rt__retrieve_buffer;

// call function (the input buffer address MUST start at the length of the buffer right before the data!)
void ESS__code__call__retrieve_buffer__explicit(ANVIL__workspace* workspace, ESS__offsets* essential_offsets, ANVIL__flag_ID flag, ANVIL__register_ID message_buffer_start_address, ANVIL__register_ID buffer_start, ANVIL__register_ID buffer_end) {
    // pass parameters
    ANVIL__code__register_to_register(workspace, flag, message_buffer_start_address, ESS__rt__retrieve_buffer__input__message_buffer_start_address);

    // call function
    ANVIL__code__call__static(workspace, flag, (*essential_offsets).offsets[ESS__ot__retrieve_buffer__start]);

    // pass outputs
    ANVIL__code__register_to_register(workspace, flag, ESS__rt__retrieve_buffer__output__buffer_start, buffer_start);
    ANVIL__code__register_to_register(workspace, flag, ESS__rt__retrieve_buffer__output__buffer_end, buffer_end);

    return;
}

// build function
void ESS__code__retrieve_buffer__explicit(ANVIL__workspace* workspace, ESS__offsets* essential_offsets) {
    // setup function offset
    (*essential_offsets).offsets[ESS__ot__retrieve_buffer__start] = ANVIL__get__offset(workspace);

    // preserve workspace
    ANVIL__code__preserve_workspace(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__preserve__START, ESS__rt__retrieve_buffer__preserve__END);

    // get parameters
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__input__message_buffer_start_address, ESS__rt__retrieve_buffer__message_buffer_start);

    // setup variables
    ANVIL__code__write_register(workspace, (ANVIL__register)(sizeof(ANVIL__length) * ANVIL__define__bits_in_byte), ESS__rt__retrieve_buffer__bit_size_of_buffer_length);
    ANVIL__code__write_register(workspace, (ANVIL__register)sizeof(ANVIL__length), ESS__rt__retrieve_buffer__byte_size_of_buffer_length);

    // get buffer length from address
    ANVIL__code__address_to_register(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__message_buffer_start, ESS__rt__retrieve_buffer__bit_size_of_buffer_length, ESS__rt__retrieve_buffer__buffer_length);

    // calculate data start address
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_add, ESS__rt__retrieve_buffer__byte_size_of_buffer_length, ESS__rt__retrieve_buffer__message_buffer_start, ANVIL__unused_register_ID, ESS__rt__retrieve_buffer__message_data_start);

    // calculate buffer end address
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_add, ESS__rt__retrieve_buffer__message_data_start, ESS__rt__retrieve_buffer__buffer_length, ANVIL__unused_register_ID, ESS__rt__retrieve_buffer__message_buffer_end);
    ANVIL__code__operate(workspace, ANVIL__sft__always_run, ANVIL__ot__integer_subtract, ESS__rt__retrieve_buffer__message_buffer_end, ANVIL__srt__constant__1, ANVIL__unused_register_ID, ESS__rt__retrieve_buffer__message_buffer_end);

    // pass outputs
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__message_data_start, ESS__rt__retrieve_buffer__output__buffer_start);
    ANVIL__code__register_to_register(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__message_buffer_end, ESS__rt__retrieve_buffer__output__buffer_end);

    // restore workspace
    ANVIL__code__restore_workspace(workspace, ANVIL__sft__always_run, ESS__rt__retrieve_buffer__preserve__START, ESS__rt__retrieve_buffer__preserve__END);

    // return to caller
    ANVIL__code__jump__explicit(workspace, ANVIL__sft__always_run, ANVIL__srt__return_address);

    return;
}

/* Package Building */
// create the library
void ESS__code__package(ANVIL__workspace* workspace, ESS__offsets* essential_offsets) {
    // write functions
    ESS__code__print_buffer(workspace, essential_offsets);
    ESS__code__retrieve_buffer__explicit(workspace, essential_offsets);

    return;
}

#endif
