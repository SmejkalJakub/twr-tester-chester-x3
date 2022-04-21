#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <twr.h>
#include <twr_chester_x3.h>
#include <twr_delay.h>
#include <bcl.h>

typedef enum
{
    TEST_I2C_0 = 0,
    TEST_I2C_1 = 1,
    TEST_DRDY_0 = 2,
    TEST_DRDY_1 = 3,
    TEST_SCALE_1 = 4,
    TEST_SCALE_2 = 5 

} test_state;

#endif
