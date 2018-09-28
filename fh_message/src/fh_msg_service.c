/**
 * fh_message_service.c
 *
 */

#include "fh_classes.h"

// instance data
struct _fh_msg_service_t {
    int filler;
};

//  Create a new msg_service
fh_msg_service_t *
fh_msg_service_new(void)
{
    fh_msg_service_t *self = (fh_msg_service_t *)calloc(1,sizeof(fh_msg_service_t));
    assert(self);
    return self;
}

//  Destroy the msg_service
void
fh_msg_service_destroy(fh_msg_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_msg_service_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}
