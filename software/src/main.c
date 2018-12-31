#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "atomic.h"
#include "systick.h"

// Structs

// Forward declarations
static void reset() __attribute__((noreturn));

static uint32_t get_free_ram();

static void gpio_init();

// Variables

// ISRs

// Functions

// TODO: REMOVE THIS
// Define the UART console
void _putchar(char ch)
{
    
}

void reset()
{
    SCB->AIRCR = 0x05FA0000 | _VAL2FLD(SCB_AIRCR_SYSRESETREQ, 1);

    while(1);
}

uint32_t get_free_ram()
{
    void *pCurrentHeap = malloc(1);

    uint32_t ulFreeRAM = (uint32_t)__get_MSP() - (uint32_t)pCurrentHeap;

    free(pCurrentHeap);

    return ulFreeRAM;
}

void gpio_init()
{

}

int init()
{
    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs

    return 0;
}
int main()
{
    while(1)
    {

    }

    return 0;
}
