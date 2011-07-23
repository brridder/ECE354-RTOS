#include "rtx_test.h"
#include "dbug.h"

//
// _DEBUG print statements describe what the tests are doing
// 

//#define _DEBUG

//
// __DEBUG print statements dump internals of tests and test infrastructure
//

//#define __DEBUG

#define GID "S11-G031"
#define TEST_DELAY_SENDER_PID 1
#define TEST_DELAY_RECEIVER_PID 2
#define TEST_MEMORY_WATCHDOG_PID 3
#define TEST_MEMORY_ALLOCATOR_PID 4
#define TEST_ERROR_CHECK_PID 5
#define TEST_MANAGEMENT_PID 6

#define TEST_SUCCESS 64
#define TEST_FAILURE 128

#define SNPRINTF_BUFFER_SIZE 256
#define FORMAT_INT_BUFFER_SIZE 16

/**
 * Basic message envelope
 */

typedef struct _message_envelope {
    unsigned char internal[64];
    unsigned char data[64];
} message_envelope;

/**
 * Globals used for string formatting
 */
 
char snprintf_buffer[SNPRINTF_BUFFER_SIZE];
char format_int_buffer[FORMAT_INT_BUFFER_SIZE];

/**
 * @brief: Copy num bytes from source to destination
 * @param: destination
 * @param: source
 * @param: num number of bytes to copy
 */

void* memcpy(void* destination, const void* source, int num) {
    int i;
    char* d;
    const char* s;

    d = (char*)destination;
    s = (const char*)source;
    for (i = 0; i < num; i++) {
        *d++ = *s++;
    }

    return destination;
}

/**
 * @brief get the length of a string
 * @param str input string
 */

int strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s-str);
}

/**
 * @brief reverses a string in place
 * @param s string to reverse
 */ 

void reverse(char s[]) {
    int i,j;
    char c;
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/**
 * @brief: converts integer into c string
 * @param: n input integer
 * @param: s character buffer to write into
 */

void itoa(int n, char s[]) {
    int i, sign;
    if ((sign = n) < 0) {
        n = -n;
    }
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while((n/=10) > 0);
    if (sign < 0) {
        s[i++] = '-';
    }
    s[i] = '\0';
    reverse(s);
}

void itox(unsigned int n, char s[]) {
    int i;
    char digit;

    i = 0;
    do {
        digit = n % 16 + '0';
        if (digit > 57) {
            digit += 7;
        }
        s[i++] = digit;
    } while((n/=16) != 0);
    s[i] = '\0';
    reverse(s);
}

/**
 * @brief: Format the input into the string, replacing %i as the input as
 an integer, and %x as the input as hex.
 * @param: buffer buffer to write to
 * @param: buffer_size maximum length to write to buffer
 * @param: format format string
 * @param: input string to replace 
 */

void snprintf_1(char* buffer, int buffer_size, const char* format, int input) {
    int index;
    const char* format_iter;
    const char* subformat_iter;

    index = 0;
    format_iter = format;
    while (*format_iter) {
        
        //
        // Look for format string combinations
        //
        
        if (*format_iter == '%') {
            format_iter++;

            if (*format_iter == 'i') {
                format_iter++;                

                //
                // Format the input as integer
                //

                itoa(input, format_int_buffer);
                subformat_iter = format_int_buffer;
                while (*subformat_iter) {
                    buffer[index] = *subformat_iter;
                    subformat_iter++;

                    index++;
                    if (index == buffer_size) {
                        goto format_done;
                    }
                }
            } else if (*format_iter == 'x') {
                format_iter++;                

                buffer[index] = '0';
                index++;                
                if (index == buffer_size) {
                    goto format_done;
                }

                buffer[index] = 'x';
                index++;
                if (index == buffer_size) {
                    goto format_done;
                }                                

                //
                // Format the input as hex
                //

                itox(input, format_int_buffer);
                subformat_iter = format_int_buffer;
                while (*subformat_iter) {
                    buffer[index] = *subformat_iter;
                    subformat_iter++;

                    index++;
                    if (index == buffer_size) {
                        goto format_done;
                    }
                }
            } else if (*format_iter == '%') {

                //
                // Literal %
                //
                
                buffer[index] = '%';

                index++;
                if (index == buffer_size) {
                    goto format_done;
                }
            }
        } else {

            //
            // Copy the format string character to the buffer
            //
            
            buffer[index] = *format_iter;
            format_iter++;

            index++;
            if (index == buffer_size) {
                goto format_done;
            }
        }
    }

    buffer[index] = '\0';

format_done:
    return;
}

/**
 * @brief: print a formatted string to JanusROM terminal
 * @param: format format string, supported substitutions are %i (integer),
 and %x (hex).
 * @param: input input to subsitute into the format string
 */

void printf_1(const char* format, int input) {
    snprintf_1(snprintf_buffer, SNPRINTF_BUFFER_SIZE, format, input);
    rtx_dbug_outs(snprintf_buffer);
}

/**
 * @brief: print a formatted string to JanusROM terminal
 */

void printf_0(const char* format) {
    printf_1(format, 0);
}

/**
 * Test globals
 */  

void test_delay_sender() {
    const int delays[] = {10, 100, 2000, 1000, 500, 1};
    const int num_messages = 6;
    int i;
    message_envelope* message;
    int sender_id;

    printf_0(GID"_test: START\r\n"GID"_test: total 3 tests\r\n");

#ifdef _DEBUG
    printf_0("Test Case 1 - Delayed send\r\n");
#endif

    //
    // Send six delayed messages (out of receive order). These messages should be
    // received in the correct order by the `test_delay_receiver()` process.
    //

    for (i = 0; i < num_messages; i++) {
        message = (message_envelope*)g_test_fixture.request_memory_block();
        memcpy(message->data, &delays[i], sizeof(delays[i]));
        g_test_fixture.delayed_send(TEST_DELAY_RECEIVER_PID, message, delays[i]);
    }

    while (1) {
        g_test_fixture.release_processor();
    } 
}

void test_delay_receiver() {
    const int delays[] = {1, 10, 100, 500, 1000, 2000};
    const int num_messages = 6;
    const int expected_successes = 7;

    int failed;
    int message_num;
    int sender_id;
    int message_delay;
    int successes;
    message_envelope* message;
    
    failed = 0;
    successes = 0;
    message_num = 0;
    while (1) 
    {
        if ((message_num < num_messages) && failed == 0) {
            message = (message_envelope*)g_test_fixture.receive_message(&sender_id);

#ifdef _DEBUG
            printf_1("  Expected message with delay %i...",
                     delays[message_num]);
#endif
            memcpy(&message_delay, message->data, sizeof(message_delay));
            if (message_delay == delays[message_num]) {
                successes++;
#ifdef _DEBUG
                printf_0("ok.\r\n");
#endif
            } else {
#ifdef _DEBUG
                printf_0("fail.\r\n");
#endif
                failed = 1;
                message->data[0] = 1;
                message->data[1] = TEST_FAILURE;
                g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            }
        
            message_num++;        
            if (message_num == num_messages) {
                successes++;

#ifdef _DEBUG
                printf_1("  Received %i messages total...success.\r\n",
                         num_messages);
#endif
            }

            if (successes == expected_successes) {

                //
                // Message the test management process with success
                //

#ifdef _DEBUG
                printf_0("  Done, sending message to management.\r\n");
#endif

                message->data[0] = 1;
                message->data[1] = TEST_SUCCESS;             
                g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            } else {
                g_test_fixture.release_memory_block(message);
            }
        } else {
            g_test_fixture.release_processor();
        }
    }

    g_test_fixture.release_processor();
}

int memory_blocks_allocated;
int memory_blocks_full;

void test_memory_watchdog() {
    void* block;
    message_envelope* message;
    int sender_id;

    //
    // This test verifies that blocked queues work as expected
    // First, allocate a single block and let the ALLOCATOR process run
    //


    
    message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
    if (sender_id == TEST_MANAGEMENT_PID) {

#ifdef _DEBUG
    printf_0("Test Case 2 - Memory allocation/blocked queue verification\r\n  Watchdog running, allocating a block...");
#endif

        memory_blocks_allocated = 0;
        memory_blocks_full = 0;

        block = g_test_fixture.request_memory_block();    
        memory_blocks_allocated++;

#ifdef _DEBUG
        printf_0("got it. Sending message to allocator.\r\n");
#endif

        g_test_fixture.send_message(TEST_MEMORY_ALLOCATOR_PID, message);
        g_test_fixture.release_processor();

        //
        // We will return here when the allocator becomes blocked. Release our 
        // block, and release processor to unblock it.
        //
    
#ifdef _DEBUG
        printf_0("  Watching running. Releasing our block...");
#endif

        memory_blocks_full = 1;
        memory_blocks_allocated--;
        g_test_fixture.release_memory_block(block);

#ifdef _DEBUG
        printf_0("done. Releasing processor.\r\n");
#endif
    } else {
        g_test_fixture.release_memory_block(message);
    }

    while(1) {
        g_test_fixture.release_processor();
    }
}

void test_memory_allocator() {
    void* blocks[64];
    message_envelope* message;
    int sender_id;
    int i;
    int j;
    int failures;

    //
    // This process will allocate as much memory as it can, until it becomes blocked;
    //

    message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
    if (sender_id == TEST_MEMORY_WATCHDOG_PID) {        
#ifdef _DEBUG
        printf_0("  Allocator running, allocating blocks.\r\n");
#endif

        i = 0;
        while (memory_blocks_full == 0) {
            blocks[i] = g_test_fixture.request_memory_block();
            memory_blocks_allocated++;
            i++;

#ifdef __DEBUG
            printf_1("    Allocated %i blocks total\r\n", memory_blocks_allocated);
#endif
        }

#ifdef _DEBUG
        printf_0("  Allocator running, cleaning up.\r\n");
#endif

        //
        // We return here when we got blocked and the watchdog ran and
        // released a block. Now we verify we can release all our blocks
        // without error.
        //

        failures = 0;

        for (j = 0; j < i; j++) {

#ifdef __DEBUG
            printf_1("    Deallocating block %i...", j);
#endif

            if (g_test_fixture.release_memory_block(blocks[j]) == RTX_ERROR) {
                failures++;
#ifdef __DEBUG
                printf_0("fail\r\n");
#endif
            } 

#ifdef __DEBUG
            else {
                printf_0("ok\r\n");
            }
#endif

        }

        //
        // Test done. Signal either success or failure, depending on how many
        // deallocations failed.
        // 

#ifdef _DEBUG
        printf_0("  Allocator done. Signaling success\r\n");
#endif

        message->data[0] = 2;
        if (failures == 0) {
            message->data[1] = TEST_SUCCESS;             
        } else {
            message->data[1] = TEST_FAILURE;  
        }

        g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
    } else {
        g_test_fixture.release_memory_block(message);
    }

    while(1) {
        g_test_fixture.release_processor();
    }
}

void test_error_check() {
    message_envelope* message;
    void* block;
    int sender_id;

    //
    // Block until the management process starts this test case by sending
    // a message.
    //

    message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
    if (sender_id == TEST_MANAGEMENT_PID) {
#ifdef _DEBUG
        printf_0("Test Case 3 - Error handling\r\n  Attempt to deallocate 0xFFFFFFFF...");
#endif
        
        block = 0xffffffff;
        if (g_test_fixture.release_memory_block(block) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Attempt to deallocate 0x00000000...");
#endif
        
        block = 0x0;
        if (g_test_fixture.release_memory_block(block) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Attempt double deallocation...");
#endif

        block = g_test_fixture.request_memory_block();
        g_test_fixture.release_memory_block(block);
        if (g_test_fixture.release_memory_block(block) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };        

#ifdef _DEBUG
        printf_0("ok\r\n  Attempt to set process priority on invalid PID...");
#endif

        if (g_test_fixture.set_process_priority(241, 1) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };


#ifdef _DEBUG
        printf_0("ok\r\n  Attempt to set process priority to invalid priority...");
#endif

        if (g_test_fixture.set_process_priority(TEST_ERROR_CHECK_PID, -11) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Attempt to get process priority of invalid PID...");
#endif

        if (g_test_fixture.get_process_priority(-132) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Sending a message to a bad PID...");
#endif

        block = g_test_fixture.request_memory_block();
        if (g_test_fixture.send_message(-79, block) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Sending delayed message to a bad PID...");
#endif

        if (g_test_fixture.delayed_send(-89, block, 1) != RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };

#ifdef _DEBUG
        printf_0("ok\r\n  Sending delayed message with negative delay...");
#endif

        if (g_test_fixture.delayed_send(TEST_ERROR_CHECK_PID, block, -100) != 
            RTX_ERROR) {
#ifdef _DEBUG
        printf_0("fail\r\n");
#endif
            message->data[0] = 3;
            message->data[1] = TEST_FAILURE;
            g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            goto test_error_check_done;
        };
        g_test_fixture.release_memory_block(block);

#ifdef _DEBUG
        printf_0("ok\r\n");
#endif

        message->data[0] = 3;
        message->data[1] = TEST_SUCCESS;
        g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
    } else {
        g_test_fixture.release_memory_block(message);
    }

 test_error_check_done:
    while(1) {
        g_test_fixture.release_processor();
    }
}

void test_management() {
    const int total_tests = 3;
    int successes;
    int failures;

    int sender_id;
    int test_case;
    int test_result;
    message_envelope* message;

    successes = 0;
    failures = 0;
    test_result = 0;
    while(1) {
        message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
        test_case = message->data[0];
        test_result = message->data[1];
        
        if (test_case == 1) {

            //
            // Begin test 2
            //

            g_test_fixture.set_process_priority(TEST_DELAY_SENDER_PID, 3);
            g_test_fixture.set_process_priority(TEST_DELAY_RECEIVER_PID, 3);
            g_test_fixture.set_process_priority(TEST_MEMORY_WATCHDOG_PID, 2);
            g_test_fixture.set_process_priority(TEST_MEMORY_ALLOCATOR_PID, 2);
            g_test_fixture.send_message(TEST_MEMORY_WATCHDOG_PID, message);
        } else if (test_case == 2) {

            //
            // Final test case finished. Release the message block.
            //

            g_test_fixture.set_process_priority(TEST_MEMORY_WATCHDOG_PID, 3);
            g_test_fixture.set_process_priority(TEST_MEMORY_ALLOCATOR_PID, 3);
            g_test_fixture.set_process_priority(TEST_ERROR_CHECK_PID, 2);

            g_test_fixture.send_message(TEST_ERROR_CHECK_PID, message);
        } else if (test_case == 3) {
            g_test_fixture.release_memory_block(message);
        }
        
        //
        // Update failure/success counters
        //

        if (test_result == TEST_SUCCESS) {
            successes++;
            printf_1(GID"_test: test %i OK\r\n", test_case);
        } else if (test_result == TEST_FAILURE) {
            failures++;
            printf_1(GID"_test: test %i FAIL\r\n", test_case);
        }

        //
        // See if we are finished the entire suite
        // 

        if ((successes + failures) == total_tests) {
            printf_1(GID"_test: %i/3 tests OK\r\n", successes);
            printf_1(GID"_test: %i/3 tests FAIL\r\n", failures);
            printf_0(GID"_test: END\r\n");
        }

#ifdef __DEBUG
        printf_1("Got a management message from case: %i\r\n", message->data[0]);
        printf_1("  Success code: %i\r\n", test_result);
        printf_1("  Successes: %i\r\n", successes);
        printf_1("  Failures: %i\r\n", failures);
#endif

    }
}

//
// Register the third party test processes with RTX
//

void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;

    printf_0("rtx_test: register_test_proc()\r\n");

    for (i = 0; i < NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
        g_test_proc[i].priority = 3;
        g_test_proc[i].sz_stack = 2048;
    }

    g_test_proc[TEST_DELAY_SENDER_PID-1].entry = test_delay_sender;
    g_test_proc[TEST_DELAY_SENDER_PID-1].priority = 2;

    g_test_proc[TEST_DELAY_RECEIVER_PID-1].entry = test_delay_receiver;
    g_test_proc[TEST_DELAY_RECEIVER_PID-1].priority = 2;

    g_test_proc[TEST_MEMORY_WATCHDOG_PID-1].entry = test_memory_watchdog;
    g_test_proc[TEST_MEMORY_ALLOCATOR_PID-1].entry = test_memory_allocator;

    g_test_proc[TEST_ERROR_CHECK_PID-1].entry = test_error_check;

    g_test_proc[TEST_MANAGEMENT_PID-1].entry = test_management;
    g_test_proc[TEST_MANAGEMENT_PID-1].priority = 1;
}

/**
 * Main entry point for this program.
 * never get invoked
 */
int main(void)
{
    printf_0("rtx_test: started\r\n");
    return 0;
}
