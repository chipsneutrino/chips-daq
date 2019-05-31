/**
 * fh_selftest.c
 *
 * Executes selftest methods.
 */

#include "../fh_classes.h"
#include "fh_selftest.h"

#define streq(s1,s2)    (!strcmp ((s1), (s2)))
 
// typedef struct {
//     const char *testname;
//     void (*test) (bool);
// } test_item_t;

static fh_test_item_t
all_tests [] = {
    { "fh_cobs", fh_cobs_test },
    { "fh_sorter", fh_sorter_test },  //todo: exclude from arm-none-eabi target due to memory contraints
    { "fh_util", fh_util_test },
    {0, 0}          //  Sentinel
};

//  -------------------------------------------------------------------------
//  Test whether a test is available.
//  Return a pointer to a test_item_t if available, NULL otherwise.
//

fh_test_item_t *
fh_test_available (const char *testname)
{
    fh_test_item_t *item;
    for (item = all_tests; item->test; item++) {
        if (streq (testname, item->testname))
            return item;
    }
    return NULL;
}

//  -------------------------------------------------------------------------
//  Run all tests.
//

void
fh_test_runall (bool verbose)
{
    fh_test_item_t *item;
    printf ("Running fh_message selftests...\n");
    for (item = all_tests; item->test; item++)
        item->test (verbose);

    printf ("Tests passed OK\n");
}
