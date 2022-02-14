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

#define rt_manip_len_max (p->fft_max / 2)
#define rt_manip_len (p->frame_size / 2)

rt_uint rt_manip_index(rt_params p, rt_uint manip_type, rt_uint frame_index)
{
  return manip_type * rt_manip_len + frame_index;
}

void rt_manip_reset(rt_params p, rt_real *manips)
{
  rt_uint i, j;
  for (i = 0; i < RT_MANIP_TYPE_COUNT; i++) {
    for (j = 0; j < rt_manip_len_max; j++) {
      rt_uint frame_index = rt_manip_index(p, i, j);
      switch (i) {
      case RT_MANIP_LEVEL:
        manips[frame_index] = 1.;
        break;
      case RT_MANIP_GATE:
        manips[frame_index] = 0.;
        break;
      case RT_MANIP_LIMIT:
        manips[frame_index] = 1.;
        break;
      default:
        fprintf(stderr, "Need to intialize all rt_manip fields.\n");
        exit(1);
        break;
      }
    }
  }
}

/**
 * @brief initializes the buffer that holds the manipulation parameter values.
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the currently active channel.
 * @return rt_real*
 *
 * Internally, the sample length of all the rt_manip buffers is N / 2, as a
 * single manip buffer will only ever refer to amplitudes OR phases.
 */
rt_real *rt_manip_init(rt_params p)
{
  rt_uint  len    = RT_MANIP_TYPE_COUNT * rt_manip_len_max;
  rt_real *manips = malloc(len * sizeof(rt_real));
  rt_manip_reset(p, manips);
  return manips;
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
 *
 * Thresholds must be adjusted by fft size / 2 to bring the amplitudes to a
 * range of -1 to 1
 */
void rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr)
{

  if (p->frame_size != p->fft_size) {
    fprintf(stderr,
            "Sorry! Zero-padding and bin manipulation not yet compatible.");
    exit(1);
  }
  rt_uint  manip_index, i;
  rt_real *manips = p->manip_multichannel ? c->manips : p->chans[0]->manips;
  /**
   * @brief Level manip section
   *
   */
  if (p->enabled_manips & RT_MANIP_LEVEL) {
    manip_index = rt_manip_index(p, RT_MANIP_LEVEL, 0);
    for (i = 0; i < rt_manip_len - 1; i++) {
      frame_ptr[i * 2] *= manips[manip_index++];
    }
    frame_ptr[1] *= manips[rt_manip_len - 1];
  }

  /**
   * @brief Gate section - lo threshold
   *
   */
  rt_real thresh_adj;
  rt_uint thresh_adj_factor = p->fft_size / 2;
  if (p->enabled_manips & RT_MANIP_GATE) {
    manip_index = rt_manip_index(p, RT_MANIP_GATE, 0);
    for (i = 0; i < rt_manip_len - 1; i++) {
      thresh_adj = (manips[manip_index++] * thresh_adj_factor);
      if (fabs(frame_ptr[i * 2]) < thresh_adj) {
        frame_ptr[i] = 0.;
      }
    }
    if (fabs(frame_ptr[1]) < (manips[manip_index] * thresh_adj_factor)) {
      frame_ptr[1] *= manips[rt_manip_len - 1];
    }
  }

  /**
   * @brief Gate section - hi threshold
   *
   */
  if (p->enabled_manips & RT_MANIP_LIMIT) {
    manip_index = rt_manip_index(p, RT_MANIP_LIMIT, 0);
    for (i = 0; i < rt_manip_len - 1; i++) {
      thresh_adj = (manips[manip_index++] * thresh_adj_factor);
      if (fabs(frame_ptr[i]) > thresh_adj) {
        frame_ptr[i] = 0.;
      }
    }
    if (fabs(frame_ptr[1]) > (manips[manip_index] * thresh_adj_factor)) {
      frame_ptr[1] *= manips[rt_manip_len - 1];
    }
  }
}
