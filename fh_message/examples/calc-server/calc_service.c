
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


#include "fh_library.h"

#include "calc_service.h"

// ------------------------------------------------------------------
// Implement an example service using the fh_message library.
//
//
// The calc service implements the following commands:
//
// [subcode] <function desc>
// payload in:
// payload out:
//
//
// [1] <add>
// in:
// int32_t[N] : numbers to add  
// out:
// int32_t : sum of numbers
//
// [2] <subtract>
// in:
// int32_t : number to subtract from
// int32_t : number to subtract  
// out:
// int32_t : difference of numbers
//
// [3] <multiply>
// in:
// int32_t[N] : numbers to multiply  
// out:
// int32_t : product of numbers
//
// [4] <divide>
// in:
// int32_t : dividend  
// int32_t : divisor  
// out:
// int32_t : quotient of numbers
//
// ------------------------------------------------------------------

// instance members
struct _calc_service_t {
    fh_service_t *service; // holds typcode/subcode function bindings
};

// create a new calc service
calc_service_t *
calc_new(uint8_t typecode)
{
    calc_service_t *self = (calc_service_t *)calloc(1, sizeof(calc_service_t));
    assert(self);

    fh_service_t *srvc = fh_service_new(typecode, self);             // new service with user-provided typecode
    fh_service_register_function(srvc, CS_ADD, &calc_add);           // bind "calc_add" function to subcode CS_ADD
    fh_service_register_function(srvc, CS_SUBTRACT, &calc_subtract); // bind "calc_subtract" function to subcode CS_SUBTRACT
    fh_service_register_function(srvc, CS_MULTIPLY, &calc_multiply); // bind "calc_multiply" functionto subcode CS_MULTIPLY
    fh_service_register_function(srvc, CS_DIVIDE, &calc_divide);     // bind "calc_divide" functionto subcode CS_DIVIDE

    self->service = srvc;

    return self;
}

//  Destroy the calc service
void
calc_destroy(calc_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        calc_service_t *self = *self_p;

        fh_service_destroy(&(self->service)); // destroy service

        free(self);
        *self_p = NULL;
    }
}

// register the service with a message distpatcher
void
calc_register_service(calc_service_t *self, fh_dispatch_t *dispatcher)
{
    fh_dispatch_register_service(dispatcher, self->service);
}

// ##################################################################
// CALC_SERVER functions
// ##################################################################
int
calc_add(void *ctx, fh_message_t *msg, fh_transport_t *transport)
{
    uint16_t len = fh_message_dataLen(msg);
    int32_t sum = 0;
    int pos = 0;
    while(pos < len)
    {
        sum += extract_int32(msg, pos);
        pos += 4;
    }

    uint8_t data[4];
    encode_int32(data, sum, 0);
    fh_message_setData(msg, data, 4);

    return fh_transport_send(transport, msg);
}

int
calc_subtract(void *ctx, fh_message_t *msg, fh_transport_t *transport)
{
    assert(fh_message_dataLen(msg) == 8);
    int32_t num1 = extract_int32(msg, 0);
    int32_t num2 = extract_int32(msg, 4);
    int32_t diff = num1 - num2;

    uint8_t data[4];
    encode_int32(data, diff, 0);
    fh_message_setData(msg, data, 4);

    return fh_transport_send(transport, msg);
}

int
calc_multiply(void *ctx, fh_message_t *msg, fh_transport_t *transport)
{
     uint16_t len = fh_message_dataLen(msg);
    int32_t prod = 0;
    int pos = 0;
    while(pos < len)
    {
        prod *= extract_int32(msg, pos);
        pos += 4;
    }

    uint8_t data[4];
    encode_int32(data, prod, 0);
    fh_message_setData(msg, data, 4);
 
    return fh_transport_send(transport, msg);
}

int
calc_divide(void *ctx, fh_message_t *msg, fh_transport_t *transport)
{
    assert(fh_message_dataLen(msg) == 8);
    int32_t num1 = extract_int32(msg, 0);
    int32_t num2 = extract_int32(msg, 4);
    int32_t diff = num1 - num2;

    uint8_t data[4];
    encode_int32(data, diff, 0);
    fh_message_setData(msg, data, 4);

    return fh_transport_send(transport, msg);
}
