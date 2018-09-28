/**
 * fh_data_access_service.h
 *
 */

#ifndef FH_DATA_ACCESS_SERVICE_H_INCLUDED
#define FH_DATA_ACCESS_SERVICE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new data_access_service
fh_data_access_service_t *
fh_data_access_service_new(void);

//  Destroy the data_access_service
void
fh_data_access_service_destroy(fh_data_access_service_t **self_p);

#ifdef __cplusplus
}
#endif

#endif
