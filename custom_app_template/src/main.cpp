#include <ch.hpp>
#include <hal.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "board/board.hpp"
#include "usb_cdc.hpp"
#include "can_bus.hpp"

// This is ugly, do something better.
//#include "../../bootloader/src/bootloader_app_interface.hpp"

namespace app
{
namespace
{
/**
 * This is the Brickproof Bootloader's app descriptor.
 * Details: https://github.com/PX4/Firmware/tree/nuttx_next/src/drivers/bootloaders/src/uavcan
 */
static const volatile struct __attribute__((packed))
{
    std::uint8_t signature[8]   = {'A','P','D','e','s','c','0','0'};
    std::uint64_t image_crc     = 0;
    std::uint32_t image_size    = 0;
    std::uint32_t vcs_commit    = GIT_HASH;
    std::uint8_t major_version  = FW_VERSION_MAJOR;
    std::uint8_t minor_version  = FW_VERSION_MINOR;
    std::uint8_t reserved[6]    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
} _app_descriptor __attribute__((section(".app_descriptor")));
}
}


class BlinkerThread : public chibios_rt::BaseStaticThread<128>
{
    void main() override
    {
        setName("blinker");
        palSetPadMode(GPIOE, 8, PAL_MODE_OUTPUT_PUSHPULL);
        while (true)
        {
            palSetPad(GPIOE,8);
            chThdSleepMilliseconds(100);
            palClearPad(GPIOE, 8);
            chThdSleepMilliseconds(100);
        }
    }

public:
    virtual ~BlinkerThread() { }
} blinker_thread_;


 static const PWMConfig pwmcfg = {
  10000,                                    //10KHz PWM clock frequency.   
  255,                                     //255 ticks is pwm resolution                                    
  NULL,
  {
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

 class BreatheThread : public chibios_rt::BaseStaticThread<128>
{
    void main() override
    {
    setName("breath");
    uint8_t brightness = 0;
    palSetPadMode(GPIOB, 1, PAL_MODE_ALTERNATE(2));
    pwmStart(&PWMD3, &pwmcfg);
    pwmEnableChannelI(&PWMD3, 3, 128);

    while (true) 
    {
        while (brightness <= 250)
        {
            pwmEnableChannelI(&PWMD3, 3, brightness++);
            chThdSleepMilliseconds(3);
        }

        while (brightness >= 5)
        {
            pwmEnableChannelI(&PWMD3, 3, brightness--);
            chThdSleepMilliseconds(3);
        }
    }
    }

public:
    virtual ~BreatheThread() { }
} breathe_thread_; 


char data[] = "Hello World !\r\n";
class HelloWorldThread : public chibios_rt::BaseStaticThread<128>
{
   void main() override
   {
      setName("Hello");
      while (true)
      {
         sdWrite(&SD1, (uint8_t *) data, strlen(data)); 
         chThdSleepMilliseconds(100);
      }
}

public:
    virtual ~HelloWorldThread() { }
} hello_world_thread_;


class LoopbackThread : public chibios_rt::BaseStaticThread<128>
{
    void main() override
    {
        setName("Loopback");
        uint8_t c;
        sdWrite(&SD1, (uint8_t *) "Give me something\r\n", strlen("Give me something\r\n")); 
        while (true) 
        {
            sdRead (&SD1, &c, 1);
            sdWrite(&SD1, (uint8_t *) "You gave me: ", strlen("You gave me: ")); 
            sdWrite(&SD1, &c, 1); 
            sdWrite(&SD1, (uint8_t *) "\r\n", 2); 
        }
    }
public:
    virtual ~LoopbackThread() { }
} loopback_thread_;


static SerialConfig uartCfg =
{
    9600, // bitrate
    0,
    0,
    0
};

int main()
{  
    halInit();
    chSysInit();
    palSetPadMode(GPIOA, 9,  PAL_MODE_ALTERNATE(7)); //Config pin PA9 for uart use
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); //Config pin PA10 for uart use
    sdStart(&SD1, &uartCfg);

	blinker_thread_.start(NORMALPRIO + 1);
    breathe_thread_.start(NORMALPRIO + 1);
    //hello_world_thread_.start(NORMALPRIO + 1);
    loopback_thread_.start(NORMALPRIO + 1);
    while(1){}
}