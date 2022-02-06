/**
 * @file rt_manip.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

rt_uint rt_manip_index(rt_params p, rt_uint manip_type, rt_uint frame_index)
{
  return manip_type * p->frame_size + frame_index;
}

/**
 * @brief initializes the buffer that holds the manipulation parameter values.
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the currently active channel.
 * @return rt_real*
 */
rt_real *rt_manip_init(rt_params p, rt_chan c)
{
  rt_uint  len  = RT_MANIP_TYPE_COUNT * p->frame_size;
  rt_real *temp = malloc(len * sizeof(rt_real));
  rt_uint  i, j;
  for (i = 0; i < RT_MANIP_TYPE_COUNT; i++) {
    for (j = 0; j < len; j++) {
      rt_uint frame_index = rt_manip_index(p, i, j);
      switch (i) {
      case RT_MANIP_LEVEL:
        temp[frame_index] = 1.;
        break;
      case RT_MANIP_CLAMP_LO:
        temp[frame_index] = 0.;
        break;
      case RT_MANIP_CLAMP_HI:
        temp[frame_index] = 1.;
        break;
      default:
        fprintf(stderr, "Need to intialize all rt_manip fields.\n");
        exit(1);
        break;
      }
    }
  }
  return temp;
}

/**
 * @brief
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the active channel.
 * !!NOTE!! when not in multichannel, this param should always be channel 0 !!
 * @param frame_ptr Pointer to the frame currently being processed.
 * This should ALWAYS be the frame of the current channel, even if multichannel
 * is not enabled.
 */
void rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr)
{

  if (p->frame_size != p->fft_size) {
    fprintf(stderr,
            "Sorry! Zero-padding and bin manipulation not yet compatible.");
    exit(1);
  }
  rt_uint manip_index, i;

  if (p->manip_settings & RT_MANIP_LEVEL) {
    manip_index = rt_manip_index(p, RT_MANIP_LEVEL, 0);
    for (i = 0; i < p->fft_size; i++) {
      frame_ptr[i] *= c->manips[manip_index++];
    }
  }

  /**
   * @brief theoretical FFT maximum amplitude for a signal with -1 to 1 range is
   * N, or the fft_size. Need to find a happy medium for the practical gating.
   *
   */
  if (p->manip_settings & RT_MANIP_CLAMP_LO) {
    manip_index = rt_manip_index(p, RT_MANIP_CLAMP_LO, i);
    for (i = 0; i < p->fft_size; i++) {
      if (fabs(frame_ptr[i]) < (c->manips[manip_index++] * p->fft_size)) {
        frame_ptr[i] = 0.;
      }
    }
  }

  if (p->manip_settings & RT_MANIP_CLAMP_HI) {
    manip_index = rt_manip_index(p, RT_MANIP_CLAMP_HI, i);
    for (i = 0; i < p->fft_size; i++) {
      if (fabs(frame_ptr[i]) > (c->manips[manip_index++] * p->fft_size)) {
        frame_ptr[i] = 0.;
      }
    }
  }
}
