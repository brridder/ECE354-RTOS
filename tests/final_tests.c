#include "rtx_test.h"
#include "dbug.h"

#define GID "S11-G031"
#define TEST_DELAY_SENDER_PID 1
#define TEST_DELAY_RECEIVER_PID 2
#define TEST_MANAGEMENT_PID 6

#define TEST_SUCCESS 64
#define TEST_FAILURE 128

#define SNPRINTF_BUFFER_SIZE 128
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

const int total_expected_successes = 1;
int total_failures;
int total_successes;

void test_delay_sender() {
    const int delays[] = {10, 100, 2000, 1000, 500, 1};
    const int num_messages = 6;
    int i;
    message_envelope* message;

    printf_0(GID"_test: START\r\n"GID"_test: total 1 test\r\n");
    total_successes = 0;

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

    int message_num;
    int sender_id;
    int message_delay;
    int successes;
    message_envelope* message;
    
    successes = 0;
    message_num = 0;
    while (1) 
    {
        if (message_num < num_messages) {
            message = (message_envelope*)g_test_fixture.receive_message(&sender_id);

#ifdef _DEBUG
            printf_1("Process 2 expected message with delay %i...",
                     delays[message_num]);
#endif
            memcpy(&message_delay, message->data, sizeof(message_delay));
            if (message_delay == delays[message_num]) {
                successes++;
#ifdef _DEBUG
                printf_0("success.\r\n");
#endif
            } else {
#ifdef _DEBUG
                printf_0("fail.\r\n");
#endif
            }
        
            message_num++;        
            if (message_num == num_messages) {
                successes++;

#ifdef _DEBUG
                printf_1("Process 2 received %i messages total...success.\r\n",
                         num_messages);
#endif
            }

            if (successes == expected_successes) {

                //
                // Message the test management process with success
                //

#ifdef _DEBUG
                printf_0("Process 2 done, sending message to management\r\n");
#endif

                message->data[0] = 1;
                message->data[1] = RTX_SUCCESS;                
                g_test_fixture.send_message(TEST_MANAGEMENT_PID, message);
            } else {
                g_test_fixture.release_memory_block(message);
            }
        }
    }

    g_test_fixture.release_processor();
}

void test3() {
    while(1) {
        g_test_fixture.release_processor();
    }
}

void test4() {
    while(1) {
        g_test_fixture.release_processor();
    }
}

void test5() {
    while(1) {
        g_test_fixture.release_processor();
    }
}

void test_management() {
    const int total_tests = 1;
    int successes;
    int failures;

    int sender_id;
    int test_result;
    message_envelope* message;

    successes = 0;
    failures = 0;
    test_result = 0;
    while(1) {
        message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
        test_result = message->data[1];
  
        if (test_result == TEST_SUCCESS) {
            successes++;
            printf_1(GID"_test: test %i OK\r\n", message->data[0]);
        } else if (test_result == TEST_FAILURE) {
            failures++;
            printf_1(GID"_test: test %i FAIL\r\n", message->data[0]);
        }

        if ((successes + failures) == total_tests) {
            printf_1(GID"_test: %i/1 tests OK\r\n", successes);
            printf_1(GID"_test: %i/1 tests FAIL\r\n", failures);
            printf_0(GID"_test: END\r\n");
        }
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
    g_test_proc[TEST_DELAY_RECEIVER_PID-1].entry = test_delay_receiver;
    g_test_proc[2].entry = test3;
    g_test_proc[3].entry = test4;
    g_test_proc[4].entry = test5;

    g_test_proc[TEST_MANAGEMENT_PID-1].entry = test_management;
    g_test_proc[TEST_MANAGEMENT_PID-1].priority = 2;
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
