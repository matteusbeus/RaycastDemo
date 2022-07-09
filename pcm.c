
#include <stdint.h>
#include "pcm.h"

// PCM chip registers
#define PCM_ENV   *((volatile uint8_t *)0xFF0001)
#define PCM_PAN   *((volatile uint8_t *)0xFF0003)
#define PCM_FDL   *((volatile uint8_t *)0xFF0005)
#define PCM_FDH   *((volatile uint8_t *)0xFF0007)
#define PCM_LSL   *((volatile uint8_t *)0xFF0009)
#define PCM_LSH   *((volatile uint8_t *)0xFF000B)
#define PCM_START *((volatile uint8_t *)0xFF000D)
#define PCM_CTRL  *((volatile uint8_t *)0xFF000F)
#define PCM_ONOFF *((volatile uint8_t *)0xFF0011)
#define PCM_WAVE  *((volatile uint8_t *)0xFF2001)

static uint8_t loop_markers[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#define BLK_PAD (32 + 255)
#define BLK_SHIFT 8

static uint8_t ChanOff;

static void pcm_cpy(uint16_t doff, void *src, uint16_t len, uint16_t conv)
{
    uint8_t *sptr = (uint8_t *)src;

    while (len > 0)
    {
        uint8_t *wptr = (uint8_t *)&PCM_WAVE;
        uint16_t woff = doff & 0x0FFF;
        uint16_t wblen = 0x1000 - woff;
        wptr += (woff << 1);

        PCM_CTRL = 0x80 + (doff >> 12); // make sure PCM chip is ON to write wave memory, and set wave bank
        pcm_delay();

        if (wblen > len)
            wblen = len;
        doff += wblen;
        len -= wblen;
        while (wblen > 0)
        {
            int16_t s = (int8_t)*sptr++;
            if (conv)
            {
                // convert from 8-bit signed samples to sign/magnitude samples
                if (s < 0)
                {
                    s = -s;
                    if (s > 127) s = 127; // clamp to -127
                }
                else
                {
                    if (s > 126) s = 126; // clamp to 126
                    s |= 128;
                }
            }
            *wptr++ = (uint8_t)(s & 255);
            wptr++;
            wblen--;
        }
    }
}

void pcm_load_samples(uint8_t start, int8_t *samples, uint16_t length)
{
    ChanOff = 0xFF;
    PCM_ONOFF = 0xFF; // turn off all channels
    pcm_cpy(start << BLK_SHIFT, samples, length, 1);
    pcm_cpy((start << BLK_SHIFT) + length, loop_markers, 32, 0);
}

uint16_t pcm_next_block(uint8_t start, uint16_t length)
{
    return start + ((length + BLK_PAD) >> BLK_SHIFT);
}

void pcm_reset(void)
{
    uint16_t i;

    ChanOff = 0xFF;
    PCM_ONOFF = 0xFF; // turn off all channels
    pcm_delay();

    for (i=0; i<8; i++)
    {
        PCM_CTRL = 0xC0 + i; // turn on pcm chip and select channel
        pcm_delay();
        PCM_ENV = 0x00; // channel env off
        pcm_delay();
        PCM_PAN = 0x00;
        pcm_delay();
        PCM_FDL = 0x00;
        pcm_delay();
        PCM_FDH = 0x00;
        pcm_delay();
        PCM_LSL = 0x00;
        pcm_delay();
        PCM_LSH = 0x00;
        pcm_delay();
        PCM_START = 0x00;
        pcm_delay();
    }
}

void pcm_set_ctrl(uint8_t val)
{
    PCM_CTRL = val;
    pcm_delay();
}

void pcm_set_off(uint8_t index)
{
    ChanOff |= (1 << index);
    PCM_ONOFF = ChanOff;
    pcm_delay();
}

void pcm_set_on(uint8_t index)
{
    ChanOff &= ~(1 << index);
    PCM_ONOFF = ChanOff;
    pcm_delay();
}

void pcm_set_start(uint8_t start, uint16_t offset)
{
    PCM_START = start + (offset >> BLK_SHIFT);
    pcm_delay();
}

void pcm_set_loop(uint16_t loopstart)
{
    PCM_LSL = loopstart & 0x00FF; // low byte
    pcm_delay();
    PCM_LSH = loopstart >> 8; // high byte
    pcm_delay();
}

void pcm_set_env(uint8_t vol)
{
    PCM_ENV = vol;
    pcm_delay();
}

void pcm_set_pan(uint8_t pan)
{
    PCM_PAN = pcm_lcf(pan);
    pcm_delay();
}

