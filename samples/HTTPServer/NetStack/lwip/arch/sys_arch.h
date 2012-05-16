/*
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIP_SYS_ARCH_H__
#define __LWIP_SYS_ARCH_H__

typedef unsigned int u32_t;

#ifdef __cplusplus
extern "C" {
#endif

//DG 2010
void sys_init(void); /* To be called first */
u32_t sys_jiffies(void); /* since power up. */

/** Returns the current time in milliseconds,
 * may be the same as sys_jiffies or at least based on it. */
u32_t sys_now(void);

#ifdef __cplusplus
}
#endif


#endif /* __LWIP_ARCH_CC_H__ */
