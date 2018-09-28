/**
 * fh_sorter.c
 *
 * Implements an N-to-1 sorter useful for combining N channels of
 * ordered records into a single stream of in-order records.
 *
 * The sorter is implemented as an inverted binary tree of nodes.
 * Each node can accept and hold a list of records in sorted order.
 * The top level nodes take in-order input from external souces
 * while the interior nodes take as input the sorted output of
 * their parent nodes.
 *
 * When a record is pushed into an input node, the subtree rooted
 * at that node is traversed and records from peer pairs are promoted
 * downwards if their ordinal value is less than the lowest value
 * record held by their peer. This comparison/promotion happens
 * recursively down the tree until one node of each peer pair is
 * empty of data. As records arrive at the terminal node they are
 * dispatched to a consumer defined by the client.
 *
 * 
 * An example instance:
 *
 *
 * +------------+   +------------+   +-------------+   +------------+
 * | node N1    |<->| node N2    |   | node N3     |<->| node N4    |
 * | (9,11,11)  |   |     ()     |   |     ()      |   |    (7)     |
 * +-----+------+   +-----+------+   +-----+-------+   +-----+------+
 *       |                |                |                 |
 *       +--------^-------+                +---------^-------+
 *                |                                  |
 *           +----v---+                         +----v---+
 *           |  sink  |< - - - - <peer> - - - ->|  sink  |
 *           | (6,8)  |                         |   ()   |
 *           +---+----+                         +---+----+
 *               |                                  |
 *               +-----------------^----------------+
 *                                 |
 *                            +----v---+
 *                            |  sink  |
 *                            |   ()   |
 *                            +--------+
 *                                 |
 *                                 v
 *                                out
 *                              (2,3,5)
 * 
 * 
 * Records will be held within the sort tree until every input channel
 * receives a record with a greater ordinal value. There is no constraint
 * on the maximum number of records that the tree may hold.  If a channel
 * is terminated (will no longer produce records), an END_OF_STREAM marker
 * with a value of UINT64_MAX will be pushed into the input node to ensure
 * that records from its peer node will be automatically promoted.
 *
 *
 * Enhancements:
 *
 *    * The number of hashmap lookups can be reduced considerably by
 *      providing a per-input-node consume() function as the return
 *      value of register(). Presuminig the caller holds the records
 *      in distinct streams.
 *
 *    * Referencing the ordinal value directly from data buffer would
 *      save some memory for large sort trees (where one channel lags the
 *      others).
 *
 *    * performace of utlist and uthash has not been evaluated against
 *      alternatives.
 *
 *
 */
#include "fh_classes.h"
#include "uthash/uthash.h"
#include "uthash/utlist.h"
#include <inttypes.h>

typedef struct _fh_sort_record_t fh_sort_record_t;
struct _fh_sort_record_t {
    uint64_t val;           // the ordering field
    uint8_t *data;          // record data
    size_t len;             // record data length
    fh_sort_record_t *prev; // supports list functions
    fh_sort_record_t *next; // supports list functions
};

typedef struct _fh_sort_node_t fh_sort_node_t;
struct _fh_sort_node_t {
    uint64_t id;                // source_id
    fh_sort_node_t *peer;       // peer node
    fh_sort_node_t *sink;       // sink node
    fh_sort_record_t *rec_list; // list of values
    UT_hash_handle hh;          // required to support hashing
    fh_sort_node_t *prev;       // supports list functions (note: list is seperate from hash)
    fh_sort_node_t *next;       // supports list functions  (note: list is seperate from hash)
};

typedef struct _fh_sort_node_list fh_sort_node_list;
struct _fh_sort_node_list {
    fh_sort_node_t *node;
    fh_sort_record_t *prev; // supports list functions
    fh_sort_record_t *next; // supports list functions
};

struct _fh_sorter_t {
    fh_sort_sink_t output;     // callback for recieving sorted records
    fh_sort_node_t *node_list; // list of all nodes (supports destruction)
    fh_sort_node_t *terminus;  // the terminal node in sort tree
    fh_sort_node_t *sort_map;  // map holding input sort nodes indexed by source id
    bool started;              // flag indicating state;
};

// forwards

// fh_sort_node
fh_sort_node_t * fh_sort_node_new(uint64_t id);
void fh_sort_node_destroy(fh_sort_node_t **self_p);
void fh_sort_node_push(fh_sort_node_t *self, fh_sort_record_t *record);
fh_sort_record_t * fh_sort_node_pop(fh_sort_node_t *self);

// fh_sort_record
fh_sort_record_t * fh_sort_record_new(uint64_t timestamp, uint8_t *data, size_t len);
void fh_sort_record_destroy(fh_sort_record_t **self_p);

// fh_sorter private functions
fh_sort_node_t * _fh_sorter_create_node(fh_sorter_t *self, uint64_t id);
bool _fh_sorter_dispatch(fh_sorter_t *self, fh_sort_record_t *record);



// ##################################################################
// # fh_sort node (internal type)
// ##################################################################

// create a new sort node
fh_sort_node_t *
fh_sort_node_new(uint64_t id)
{
    fh_sort_node_t *self = (fh_sort_node_t *)calloc(1, sizeof(fh_sort_node_t));
    assert(self);
    self->id = id;
    
    return self;
}

// destroy a sort node
void
fh_sort_node_destroy(fh_sort_node_t **self_p)
{

    assert(self_p);
    if (*self_p) {
        fh_sort_node_t *self = *self_p;

        // Note: Normally one should not destroy a sorter that
        //       is holding data ... but if so, free the data.
        //       If the sorter was used to completion, the nodes
        //       will hold the EOS record.
        fh_sort_record_t *elt, *tmp;
        DL_FOREACH_SAFE(self->rec_list, elt, tmp)
        {
            DL_DELETE(self->rec_list, elt);
            if (elt->data) {
                free(elt->data);
            }
            fh_sort_record_destroy(&elt);
            free(elt);
        }
        free(self);
    }

    *self_p = NULL;
}

// push a record to a node
void
fh_sort_node_push(fh_sort_node_t *self, fh_sort_record_t *record)
{
    //1. append data record to node
    DL_APPEND(self->rec_list, record);

    //2. descend the sub-tree rooted at this node, promoting records towards the terminal
    // node as appropriate

    // bail at terminal node
    if(self->sink == NULL)
    {
        return;
    }

    while(self->rec_list && self->peer->rec_list)
    {
        if(self->rec_list->val > self->peer->rec_list->val)
        {
             // push peer head value to sink
             fh_sort_node_push(self->sink, fh_sort_node_pop(self->peer));
        }
        else
        {
            // push head value to sink
             fh_sort_node_push(self->sink, fh_sort_node_pop(self));
        }
    }

}

// pop a record from a node
fh_sort_record_t *
fh_sort_node_pop(fh_sort_node_t *self)
{
    fh_sort_record_t *ret = self->rec_list;
    DL_DELETE(self->rec_list, self->rec_list);
    return ret;
}

void
fh_sort_node_print_info(fh_sort_node_t *self, FILE *out)
{
    fh_sort_record_t *rec;
fprintf(out, "Node {");
DL_FOREACH(self->rec_list, rec) {
        fprintf(out, "%"PRId64",", rec->val);
    }
    fprintf(out, "}\n");
}

// ##################################################################
// # fh_sort record (internal type)
// ##################################################################

// create a new sort record.
// Note: fh_sort_record does not assume ownership of data memory.
fh_sort_record_t *
fh_sort_record_new(uint64_t ordinal, uint8_t *data, size_t len)
{
    fh_sort_record_t *self = (fh_sort_record_t *)calloc(1, sizeof(fh_sort_record_t));
    assert(self);

    self->val = ordinal;
    self->data = data;
    self->len = len;
    return self;
}

// destroy a sort record
void
fh_sort_record_destroy(fh_sort_record_t **self_p)
{
    // NOTE: self->data is not freed when the record is destroyed.
    //       since the fh_sort_record_t is only a temoprary holder of this
    //       data while it is held in the sorter.

    assert(self_p);
    if (*self_p) {
        fh_sort_record_t *self = *self_p;

        free(self);
        *self_p = NULL;
    }
}

// ##################################################################
// # fh_sorter (public type)
// ##################################################################

// create a sorter
fh_sorter_t *
fh_sorter_new(fh_sort_sink_t output)
{
    fh_sorter_t *self = (fh_sorter_t *)calloc(1, sizeof(fh_sorter_t));
    assert(self);
    self->output = output;
    self->node_list = NULL;
    self->sort_map = NULL;
    self->started = false;

    return self;
}

// destroy a sorter
void
fh_sorter_destroy(fh_sorter_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_sorter_t *self = *self_p;

        // delete the hashmap of nodes indexed by source id
        HASH_CLEAR(hh, self->sort_map);

        // destroy all the nodes ... this includes
        // input nodes, internal nodes and the terminal node
        fh_sort_node_t *node, *tmp;
        DL_FOREACH_SAFE(self->node_list, node, tmp)
        {
            DL_DELETE(self->node_list, node);
            fh_sort_node_destroy(&node);
        }

        free(self);
    }

    *self_p = NULL;
}

// register a source with an input channel. All channels
// must be registered with a unique source id before starting
// the sorter.
void
fh_sorter_register(fh_sorter_t *self, uint64_t source_id)
{
    fh_sort_node_t *node = _fh_sorter_create_node(self, source_id);

    // since this is an input node, store in hash map indexed by source id
    HASH_ADD(hh, self->sort_map, id, 8,  node); 
}

// start the sorter
// must be called before pushing any records to the sorter
// all input nodes must be registered before calling start.
void
fh_sorter_start(fh_sorter_t *self)
{
    // Note: At this point all of the nodes in the node list are input nodes
    //       that were created via the register() function.
    //
    //       While building the sort tree, this function will append interior
    //       nodes on to the list.
    uint16_t num_channels;
    fh_sort_node_t *node;
    DL_COUNT(self->node_list, node, num_channels);

   // build the sort tree
    fh_sort_node_t *yyy[num_channels]; //tmp node array
    fh_sort_node_t *zzz[num_channels]; //tmp node array
    fh_sort_node_t **unconnected = yyy;
    fh_sort_node_t **tmp = zzz;

     // init unconnected with input nodes
    int remaining = 0;
    DL_FOREACH(self->node_list, node) {
        unconnected[remaining++] = node;
    }

    while (remaining > 1) {
        // reset storage indexes each round
         int unconnected_idx = 0;
         int tmp_idx = 0;
         while(remaining > 1) 
         {
           fh_sort_node_t *left = unconnected[unconnected_idx++];
           fh_sort_node_t *right = unconnected[unconnected_idx++];
           remaining--;
           remaining--;

           fh_sort_node_t *sink = _fh_sorter_create_node(self, 0);

           left->peer = right;
           right->peer = left;
           left->sink = sink;
           right->sink = sink;

           tmp[tmp_idx++] = sink;
         }

         // an odd node will be promoted to next level
         if(remaining == 1)
         {
            tmp[tmp_idx++] = unconnected[unconnected_idx];
         }

         // reverse the working arrays
         unconnected = tmp;
         tmp = unconnected;
         remaining = tmp_idx;
    }

    // the final unconnected node is the terminal node
    assert(remaining == 1);
    self->terminus = unconnected[0];

    self->started = true;

}

// Accept a record into the sorter
// Note: ordinal is typically a timestamp but may be any quantity that defines the
//       ordering of the records.
bool
fh_sorter_consume(fh_sorter_t *self, uint64_t ordinal, uint64_t source_id, void *data, uint32_t len)
{
    assert(self->started);

    //NOTE: The calling thread is used to perform the sort work
    //      and dispatch.  An alternative is to utilize a separate
    //      thread to reduce the latency seen by the caller.

    //1. push data into sort tree
    fh_sort_node_t *node;
    // HASH_FIND(hh, self->sort_map->nodes, &source_id, 8, node);
    HASH_FIND(hh, self->sort_map, &source_id, 8, node);

    if (!node) {
        // WARNING: unregistered channel
        return false;
    }

    // printf("pushing %llu to input node %llu\n", ordinal, source_id);
    fh_sort_node_push(node, fh_sort_record_new(ordinal, data, len));

    // 2. dispatch sorted records accumulated at the terminal node
    fh_sort_record_t *elt, *tmp;
    DL_FOREACH_SAFE(self->terminus->rec_list, elt, tmp) {
      DL_DELETE(self->terminus->rec_list, elt);

      _fh_sorter_dispatch(self, elt);
    }

    return true;
}

// terminate input from a stream
bool
fh_sorter_eos(fh_sorter_t *self, uint64_t source_id)
{
    assert(self->started);

    return fh_sorter_consume(self, EOS_MARKER, source_id, NULL, 0);
}

fh_sort_node_t *
_fh_sorter_create_node(fh_sorter_t *self, uint64_t id)
{
    // create a new sort node, storing in node list for later deletion
    fh_sort_node_t *node = fh_sort_node_new(id);

     DL_APPEND(self->node_list, node);

    return node;
}

// dispatch a sorted record to the callback function
bool 
_fh_sorter_dispatch(fh_sorter_t *self, fh_sort_record_t *record)
{
    // NOTE: ownership of the record data memory is transfered
    //       to the recipient
    bool ret =  (*(self->output.callback))(self->output.ctx, record->val, record->data, record->len);
    fh_sort_record_destroy(&record);
    return ret;

}

// print some diagnostic info about the state of the sort tree
void
fh_sorter_print_info(fh_sorter_t *self, FILE *out)
{
    fh_sort_node_t *node;
    int i = 0;
    DL_FOREACH(self->node_list, node) {
    //     unconnected[remaining++] = node;
        fprintf(out, "Node [%d]: ", i++);
        fh_sort_node_print_info(node, out);
    }
}

//###################################################################
//# self test support
//###################################################################

// models the recipient of sorted test records
typedef struct {
    uint64_t first_val;
    uint64_t last_val;
    uint64_t num_rcv;
    bool ordered;
} test_sink_t;


test_sink_t *
test_sink_new()
{
    test_sink_t *self = (test_sink_t *)calloc(1, sizeof(test_sink_t));
     self->first_val = EOS_MARKER;
    self->last_val = 0;
    self->num_rcv = 0;
    self->ordered = true;

    return self;
}

void
test_sink_destroy(test_sink_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        test_sink_t *self = *self_p;
        free(self);
    }

    *self_p = NULL;
}

bool
test_sink_consume(void *ctx, uint64_t val, void *data, uint32_t len)
{
    if (val == EOS_MARKER) {
        return true;
    }

    test_sink_t *self = (test_sink_t *)ctx;

    if (self->first_val == EOS_MARKER) {
        self->first_val = val;
    }

    assert(self->last_val <= val);

    if (self->last_val > val) {
        self->ordered = false;
    }

    self->last_val = val;
    self->num_rcv++;

    // some test send {val, source_id} in the payload
    if (data) {
        uint64_t enc_val = *(uint64_t *)data;
        //    uint64_t enc_src = *((uint64_t *)data + 1);
        // printf("Got sorted record %llu from source %llu\n", enc_val, enc_src);
        assert(enc_val == val);
        free(data);
    }
    return true;
}

// models a test record source
typedef struct {
    uint64_t source_id;
    fh_rand_ord_seq_t *seq;
    uint32_t num_records_issued;
} test_source_t;

test_source_t *
test_source_new(uint64_t id)
{
    static int seed = 9999; // unique seed per instance
    seed += 27;

    test_source_t *self = (test_source_t*)calloc(1, sizeof(test_source_t));

    self->source_id = id;
    self->seq = fh_rand_ord_seq_new(seed, 0, 1000);
    self->num_records_issued = 0;

    return self;
}

void
test_source_destroy(test_source_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        test_source_t *self = *self_p;

        fh_rand_ord_seq_destroy(&(self->seq));
        free(self);
    }

    *self_p = NULL;
}

// self test
void
fh_sorter_test(bool verbose)
{

    printf(" * fh_sorter: ");

    //
    // test contructor/destructor
    //
    {
        for (int i = 1; i < 16; i++) {
            fh_sorter_t *s = fh_sorter_new((fh_sort_sink_t){NULL, NULL});
            for (int j = 0; j < i; j++) {
                fh_sorter_register(s, i);
            }

            fh_sorter_start(s);
            fh_sorter_destroy(&s);
            assert(s == NULL);
        }
    }

    //
    // test a trivial sort that ends with this state:
    // +------------+   +------------+   +-------------+   +------------+
    // | node N1    |<->| node N2    |   | node N3     |<->| node N4    |
    // | (9,11,11)  |   |     ()     |   |     ()      |   |    (7)     |
    // +-----+------+   +-----+------+   +-----+-------+   +-----+------+
    //       |                |                |                 |
    //       +--------^-------+                +---------^-------+
    //                |                                  |
    //           +----v---+                         +----v---+
    //           |  sink  |< - - - - <peer> - - - ->|  sink  |
    //           | (6,8)  |                         |   ()   |
    //           +---+----+                         +---+----+
    //               |                                  |
    //               +-----------------^----------------+
    //                                 |
    //                            +----v---+
    //                            |  sink  |
    //                            |   ()   |
    //                            +--------+
    //                                 |
    //                                 v
    //                                out
    //                              (2,3,5)
    //
    {
        uint64_t ch1_data[] = {2, 9, 11, 11};
        uint64_t ch2_data[] = {6, 8};
        uint64_t ch3_data[] = {5};
        uint64_t ch4_data[] = {3, 7};

        test_sink_t *sink = test_sink_new();

        fh_sorter_t *s = fh_sorter_new((fh_sort_sink_t){sink, &test_sink_consume});
        fh_sorter_register(s, 1);
        fh_sorter_register(s, 2);
        fh_sorter_register(s, 3);
        fh_sorter_register(s, 4);

        fh_sorter_start(s);

        fh_sorter_consume(s, ch1_data[0], 1, NULL, 0);
        fh_sorter_consume(s, ch1_data[1], 1, NULL, 0);
        fh_sorter_consume(s, ch1_data[2], 1, NULL, 0);
        fh_sorter_consume(s, ch1_data[3], 1, NULL, 0);

        fh_sorter_consume(s, ch2_data[0], 2, NULL, 0);
        fh_sorter_consume(s, ch2_data[1], 2, NULL, 0);

        fh_sorter_consume(s, ch3_data[0], 3, NULL, 0);

        fh_sorter_consume(s, ch4_data[0], 4, NULL, 0);
        fh_sorter_consume(s, ch4_data[1], 4, NULL, 0);

        // fh_sorter_print_info(s, stdout);

        assert(sink->num_rcv == 3);
        assert(sink->first_val == 2);
        assert(sink->last_val == 5);

        fh_sorter_destroy(&s);
        test_sink_destroy(&sink);
    }

    //
    // sort a 100K values from 128 channels
    //
    uint16_t num_inputs = 128;
    uint32_t num_values = 100000;

    test_sink_t *sink = test_sink_new();
    fh_sorter_t *sorter = fh_sorter_new((fh_sort_sink_t){sink, &test_sink_consume});

    test_source_t *sources[num_inputs];
    for (int i = 0; i < num_inputs; i++) {
        sources[i] = test_source_new(i);
        fh_sorter_register(sorter, sources[i]->source_id);
    }

    fh_sorter_start(sorter);

    uint64_t min_value = EOS_MARKER;
    uint64_t max_value = 0;
    uint32_t num_sent = 0;
    uint16_t max_span = 1000;
    bool done = false;
    if (verbose) {
        printf("Sorting %"PRIu32" records from %d sources\n", num_values, num_inputs);
    }
    while (!done) {
        done = true;
        for (int i = 0; i < num_inputs; i++) {
            test_source_t *src = sources[i];
            int to_send = max_span;
            uint32_t remaining = num_values - src->num_records_issued;
            if (remaining < max_span) {
                to_send = remaining;
            }
            for (int j = 0; j < to_send; j++) {

                uint64_t val = rand_ord_seq_next(src->seq);
                // printf("pushing %llu from %llu\n", val, src->source_id);

                char *buf = calloc(1, 18);
                *((uint64_t *)buf) = val;
                *(((uint64_t *)buf) + 1) = src->source_id;

                assert(fh_sorter_consume(sorter, val, src->source_id, buf, 8));

                src->num_records_issued++;
                num_sent++;
                if (val < min_value) {
                    min_value = val;
                }
                if (val > max_value) {
                    max_value = val;
                }
            }
            if (src->num_records_issued < num_values) {
                done = false;
            }
        }
    }

    // terminate all channels
    for (int i = 0; i < num_inputs; i++) {
        fh_sorter_eos(sorter, sources[i]->source_id);
    }

    if (verbose) {
        printf("Pushed %" PRIu32 " records into sorter values [%"PRId64" - %"PRId64"]\n", num_sent, min_value, max_value);
        printf("Sorter test sink received %"PRId64" records values [%"PRId64" - %"PRId64"]\n", sink->num_rcv, sink->first_val,
               sink->last_val);
    }
    assert(sink->ordered);
    assert(sink->num_rcv == (num_values * num_inputs));
    assert(sink->last_val == max_value);

    // cleanup
    fh_sorter_destroy(&sorter);
    test_sink_destroy(&sink);
    for (int i = 0; i < num_inputs; i++) {
        test_source_destroy(&sources[i]);
    }

    printf("OK\n");
}
