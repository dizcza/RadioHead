// RH_L0RA.cpp
//
// Copyright (C) 2020 Mike McCauley
// $Id: RH_L0RA.cpp,v 1.1 2022/01/26 23:39:39 mikem Exp $

#if (RH_PLATFORM == RH_PLATFORM_STM32L0) && (defined (STM32L082xx) || defined (STM32L072xx) || defined (STM32L052xx))

#include <RH_L0RA.h>

// Pointer to the _only_ permitted ABZ instance (there is only one radio connected to this device)
RH_L0RA* RH_L0RA::_thisDevice;

// The muRata cmwx1zzabz module has its builtin SX1276 radio connected to the processor's SPI1 port,
// but the Arduino compatible SPI interface in Grumpy Pizzas Arduino Core is configured for SPI1 or SPI2
// depending on the exact board variant selected.
// So here we define our own Arduino compatible SPI interface
// so we are _sure_ to get the one connected to the radio, independent of the board variant selected
#include <stm32l0_spi.h>
static const stm32l0_spi_params_t RADIO_SPI_PARAMS = {
    RADIO_SPI_INSTANCE,
    0,
    STM32L0_DMA_CHANNEL_NONE,
    STM32L0_DMA_CHANNEL_NONE,
    {
        RADIO_MOSI,
        RADIO_MISO,
        RADIO_SCLK,
        STM32L0_GPIO_PIN_NONE,
    },
};

// Create and configure an Arduino compatible SPI interface. This will be referred to in RHHardwareSPI.cpp
// and used as the SPI interface to the radio.
static stm32l0_spi_t RADIO_SPI;
SPIClass radio_spi(&RADIO_SPI, &RADIO_SPI_PARAMS);

// Glue code between the 'C' DIO0 interrupt and the C++ interrupt handler in RH_RF95
void RH_INTERRUPT_ATTR RH_L0RA::isr()
{
    _thisDevice->handleInterrupt();
}

RH_L0RA::RH_L0RA():
    RH_RF95(RH_INVALID_PIN, RH_INVALID_PIN)
{
}

bool RH_L0RA::init()
{
    _thisDevice = this;

    // REVISIT: RESET THE RADIO???
    
    // The SX1276 radio DIO0 is connected to STM32L0xx pin RADIO_DIO_0
    // It will later be configured as an interrupt
    stm32l0_gpio_pin_configure(RADIO_DIO_0,     (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_PULLDOWN | STM32L0_GPIO_OSPEED_HIGH | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_INPUT));

    // Here we configure the interrupt handler for DIO0 to call the C++
    // interrupt handler in RH_RF95, in a roundabout way
#ifdef STM32L0_EXTI_CONTROL_PRIORITY_CRITICAL
    stm32l0_exti_attach(RADIO_DIO_0, (STM32L0_EXTI_CONTROL_PRIORITY_CRITICAL | STM32L0_EXTI_CONTROL_EDGE_RISING), (stm32l0_exti_callback_t)isr, NULL); // STM32L0_EXTI_CONTROL_PRIORITY_CRITICAL not in 0.0.10
#else
    stm32l0_exti_attach(RADIO_DIO_0, STM32L0_EXTI_CONTROL_EDGE_RISING, (stm32l0_exti_callback_t)isr, NULL);
#endif
    // The SX1276 radio slave select (NSS) is connected to different STM32 pins in each module.
    // We use native STM32 calls because the various different variants in the Grumpy Pizza
    // Arduino core and various forks of that core have inconsistent definitions of the Arduino compatible
    // pins. We want to be sure we get the right ones for the muRata modules connections to the Radio
    stm32l0_gpio_pin_configure(RADIO_NSS, (STM32L0_GPIO_PARK_HIZ | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_HIGH | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
    
#if defined (ARDUINO_LGT92) || defined (ARDUINO_K92_L072Z)
    // RFM95 module has an antenna switch which is driven by SX1276 xx pin the right way to connect the antenna
    // to the appropriate SX1276 RX or PA_BOOST pins respecitvely.
#elif defined (ARDUINO_T_IMPULSE) || defined (ARDUINO_K48_S76G)  || defined (ARDUINO_K48_S76G_SWD) || defined (ARDUINO_K48_S76G_I2C) || defined (ARDUINO_K76_S76S)
    // AcSIP S76G module has a SE2435L 860 to 930 MHz FEM which must be driven the right way to connect the antenna
    // to the appropriate SX1276 pins. STM32L0_GPIO_PIN_PA1 selects RX or PA_BOOST respecitvely.
    // See modeWillChange() for implementation of pin twiddling when the transmitter is on
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_RX, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
#elif defined (ARDUINO_RHF76_052) || defined (ARDUINO_K52_RHF76_052)
    // RHF76 module has two switches for LF and HF antennas which must be driven the right way to connect the antenna
    // to the appropriate SX1276 pins.
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_RX,     (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_TX_RFO, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
#else
    // muRata cmwx1zzabz module has an antenna switch which must be driven by the right way to connect the antenna
    // to the appropriate SX1276 pins.
    // Antenna switch might be something like NJG180K64, but not sure.
    // See Application note: AN-ZZABZ-001 P. 20/20
    // in typeABZ_hardware_design_guide_revC.pdf
    // with 3 pins connected to  STM32L0_GPIO_PIN_PA1, STM32L0_GPIO_PIN_PC2, STM32L0_GPIO_PIN_PC1
    // which select RX, RFO or PA_BOOST respecitvely
    // See modeWillChange() for implementation of pin twiddling when the transmitter is on
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_RX,       (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_TX_RFO,   (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
    stm32l0_gpio_pin_configure(RADIO_ANT_SWITCH_TX_BOOST, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_NONE | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
#endif
    return RH_RF95::init();
}

bool RH_L0RA::deinit()
{
    setModeIdle();
    stm32l0_exti_detach(RADIO_DIO_0);
    return true;
}

void  RH_L0RA::selectSlave()
{
    stm32l0_gpio_pin_write(RADIO_NSS, 0);
}

void  RH_L0RA::deselectSlave()
{
   stm32l0_gpio_pin_write(RADIO_NSS, 1);
}
    
bool RH_L0RA::modeWillChange(RHMode mode)
{
    if (mode == RHModeTx)
    {
#if defined (ARDUINO_LGT92) || defined (ARDUINO_K92_L072Z)
    //RFM95 uses its own pin to switch TX/RX FEM
#elif defined (ARDUINO_T_IMPULSE) || defined (ARDUINO_K48_S76G)  || defined (ARDUINO_K48_S76G_SWD) || defined (ARDUINO_K48_S76G_I2C) || defined (ARDUINO_K76_S76S)
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 0);
#elif defined (ARDUINO_RHF76_052) || defined (ARDUINO_K52_RHF76_052)
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 0);
	// Tell the antenna switch to connect to one of the transmitter output pins
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_RFO, _useRFO ? 1 : 0);
#else
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 0);
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_RFO, _useRFO ? 1 : 0);
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_BOOST, _useRFO ? 0 : 1);
#endif
    }
    else
    {
#if defined (ARDUINO_LGT92) || defined (ARDUINO_K92_L072Z)
#elif defined (ARDUINO_T_IMPULSE) || defined (ARDUINO_K48_S76G)  || defined (ARDUINO_K48_S76G_SWD) || defined (ARDUINO_K48_S76G_I2C) || defined (ARDUINO_K76_S76S)
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 1);
#elif defined (ARDUINO_RHF76_052) || defined (ARDUINO_K52_RHF76_052)
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 1);
	// Enabling the RX from the antenna switch improves reception RSSI by about 5
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_RFO, 0);
#else
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_RX, 1);
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_RFO, 0);
	stm32l0_gpio_pin_write(RADIO_ANT_SWITCH_TX_BOOST, 0);
#endif
    }
    return true;
}

#endif
