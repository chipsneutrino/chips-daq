/**
 * fh_slow_ctrl_service.h
 *
 */

#ifndef FH_SLOW_CTRL_SERVICE_H_INCLUDED
#define FH_SLOW_CTRL_SERVICE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new slow_ctrl_service
fh_slow_ctrl_service_t *
fh_slow_ctrl_service_new(void);

//  Destroy the slow_ctrl_service
void
fh_slow_ctrl_service_destroy(fh_slow_ctrl_service_t **self_p);

#ifdef __cplusplus
}
#endif

#endif
