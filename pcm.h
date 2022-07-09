#ifndef _PCM_H
#define _PCM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* from pcm.c */
extern void pcm_load_samples(uint8_t start, int8_t *samples, uint16_t length);
extern uint16_t pcm_next_block(uint8_t start, uint16_t length);
extern void pcm_reset(void);
extern void pcm_set_ctrl(uint8_t val);
extern void pcm_set_off(uint8_t index);
extern void pcm_set_on(uint8_t index);
extern void pcm_set_start(uint8_t start, uint16_t offset);
extern void pcm_set_loop(uint16_t loopstart);
extern void pcm_set_env(uint8_t vol);
extern void pcm_set_pan(uint8_t pan);
/* from pcm-io.s */
extern uint8_t pcm_lcf(uint8_t pan);
extern void pcm_delay(void);
extern void pcm_set_period(uint32_t period);
extern void pcm_set_freq(uint32_t freq);
extern void pcm_set_timer(uint16_t bpm);
extern void pcm_stop_timer(void);
extern void pcm_start_timer(void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif
