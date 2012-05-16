#include "sys_arch.h"
#include "mbed.h"
//DG 2010

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __COMPLEX_AND_BUGGUY_HANDLING__
static Timer* pTmr = NULL;

void sys_init(void)
{
  //Start Timer
  pTmr = new Timer();
  pTmr->start();
}

u32_t sys_jiffies(void) /* since power up. */
{
  return (u32_t) (pTmr->read_ms()/10); //In /10ms units
}

u32_t sys_now(void)
{
  return (u32_t) pTmr->read_ms(); //In /ms units
}
#elif0
void sys_init(void)
{

}

u32_t sys_jiffies(void) /* since power up. */
{
  static int count = 0;
  return ++count;
}

u32_t sys_now(void)
{
  return (u32_t) time(NULL);
}
#else
static Timer* pTmr = NULL;

void sys_init(void)
{
  //Start Timer
  pTmr = new Timer();
  pTmr->start();
}

u32_t sys_jiffies(void) /* since power up. */
{
  static int count = 0;
  return ++count;
  //return (u32_t) (pTmr->read_us());
}

u32_t sys_now(void)
{
  return (u32_t) (pTmr->read_ms()); //In /ms units
}
#endif


#ifdef __cplusplus
}
#endif
