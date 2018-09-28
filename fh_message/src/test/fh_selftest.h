/**
 * fh_selftest.h
 *
 * Executes selftest methods.
 */

#ifndef FH_SELFTEST_H_INCLUDED
#define FH_SELFTEST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define streq(s1,s2)    (!strcmp ((s1), (s2)))
 
typedef struct {
    const char *testname;
    void (*test) (bool);
} fh_test_item_t;



//  Test whether a test is available.
//  Return a pointer to a test_item_t if available, NULL otherwise.
fh_test_item_t *
fh_test_available (const char *testname);

//  Run all tests.
void
fh_test_runall (bool verbose);

#endif
