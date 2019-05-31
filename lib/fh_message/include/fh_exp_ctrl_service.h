/**
 * fh_exp_ctrl_service.h
 *
 */

#ifndef FH_EXP_CTRL_SERVICE_H_INCLUDED
#define FH_EXP_CTRL_SERVICE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new exp_ctrl_service
fh_exp_ctrl_service_t *
fh_exp_ctrl_service_new(void);

//  Destroy the exp_ctrl_service
void
fh_exp_ctrl_service_destroy(fh_exp_ctrl_service_t **self_p);

#ifdef __cplusplus
}
#endif

#endif
