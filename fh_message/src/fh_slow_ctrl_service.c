/**
 * fh_slow_ctrl_service.c
 *
 */

#include "fh_classes.h"

// instance data
struct _fh_slow_ctrl_service_t {
    int filler;
};

//  Create a new slow_ctrl_service
fh_slow_ctrl_service_t *
fh_slow_ctrl_service_new(void)
{
    fh_slow_ctrl_service_t *self = (fh_slow_ctrl_service_t *)calloc(1,sizeof(fh_slow_ctrl_service_t));
    assert(self);
    return self;
}

//  Destroy the slow_ctrl_service
void
fh_slow_ctrl_service_destroy(fh_slow_ctrl_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_slow_ctrl_service_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}
