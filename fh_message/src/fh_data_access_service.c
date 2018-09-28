/**
 * fh_data_access_service.c
 *
 */

#include "fh_classes.h"

// instance data
struct _fh_data_access_service_t {
    int filler;
};

//  Create a new data_access_service
fh_data_access_service_t *
fh_data_service_new(void)
{
    fh_data_access_service_t *self = (fh_data_access_service_t *)calloc(1, sizeof(fh_data_access_service_t));
    assert(self);
    return self;
}

//  Destroy the data_access_service
void
fh_data_service_destroy(fh_data_access_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_data_access_service_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}
