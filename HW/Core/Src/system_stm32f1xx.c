/**
  * @file    system_stm32f1xx.c
  * @brief   STM32F1xx system source file
  *
  * Configures SystemCoreClock = 72 MHz using HSE(8MHz) PLL x9.
  * If HSE fails, falls back to HSI(8MHz)/2 PLL x16 = 64 MHz.
  */

#include "stm32f1xx.h"

/* ---------------------------------------------------------------------------
 * Exported variables
 * --------------------------------------------------------------------------*/
uint32_t SystemCoreClock = 72000000UL;

const uint8_t AHBPrescTable[16]  = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]   = {0, 0, 0, 0, 1, 2, 3, 4};

/* ---------------------------------------------------------------------------
 * SystemInit
 *   Called before main(). Sets up:
 *     - HSE ON → PLL x9 → SYSCLK = 72 MHz
 *     - AHB  div1  → HCLK  = 72 MHz
 *     - APB1 div2  → PCLK1 = 36 MHz  (I2C1 source)
 *     - APB2 div1  → PCLK2 = 72 MHz  (TIM1, GPIOA/B source)
 *     - Flash 2 wait states
 * --------------------------------------------------------------------------*/
void SystemInit(void)
{
    /* FPU not present on Cortex-M3 */

    /* ===== 1. Enable HSI (fallback) and reset RCC to safe state ===== */
    RCC->CR |= 0x00000001U;            /* HSI ON */

    /* Reset CFGR */
    RCC->CFGR = 0x00000000U;

    /* Reset HSEON, CSSON, PLLON */
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);

    /* Reset PLLCFGR (PLLSRC, PLLXTPRE, PLLMUL) */
    RCC->CFGR &= 0xFF80FFFFU;

    /* Disable all clock interrupts */
    RCC->CIR = 0x00000000U;

    /* ===== 2. Enable HSE ===== */
    RCC->CR |= RCC_CR_HSEON;

    /* Wait until HSE is ready (with timeout) */
    volatile uint32_t timeout = 0x500;
    while (!(RCC->CR & RCC_CR_HSERDY) && --timeout);

    if (timeout == 0)
    {
        /* HSE failed — stay on default HSI */
        SystemCoreClock = 8000000UL;
        return;
    }

    /* ===== 3. Configure Flash prefetch + 2 wait states ===== */
    /* Flash: 预取缓冲使能(bit4) + 2等待周期(bits[2:0]=010)
     * FLASH_ACR_LATENCY_2 在某些CMSIS版本中不存在，使用数值 0x12U：
     * bit4(PRFTBE)=1 | bits[1:0]=10(2 wait states) */
    FLASH->ACR = (FLASH->ACR & ~0x07U) | 0x12U;

    /* ===== 4. AHB/APB prescalers ===== */
    /* HCLK  = SYSCLK / 1  */
    /* PCLK2 = HCLK   / 1  */
    /* PCLK1 = HCLK   / 2  */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1
              |  RCC_CFGR_PPRE2_DIV1
              |  RCC_CFGR_PPRE1_DIV2;

    /* ===== 5. PLL: source = HSE, multiplier = x9 → 72 MHz ===== */
    RCC->CFGR |= RCC_CFGR_PLLSRC            /* PLL source = HSE */
              |  RCC_CFGR_PLLMULL9;          /* PLL x9           */

    /* ===== 6. Enable and wait for PLL ===== */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* ===== 7. Switch system clock to PLL ===== */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |=  RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClock = 72000000UL;
}

/* ---------------------------------------------------------------------------
 * SystemCoreClockUpdate — re-reads actual frequency from registers
 * --------------------------------------------------------------------------*/
void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllmull = 0, pllsource = 0;

    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
        case 0x00:  /* HSI as system clock */
            SystemCoreClock = 8000000UL;
            break;
        case 0x04:  /* HSE as system clock */
            SystemCoreClock = 8000000UL;
            break;
        case 0x08:  /* PLL as system clock */
            pllmull   = (RCC->CFGR & RCC_CFGR_PLLMULL) >> 18;
            pllsource = (RCC->CFGR & RCC_CFGR_PLLSRC)  >> 16;
            pllmull   = (pllmull == 0x0F) ? 16UL : (pllmull + 2UL);
            if (pllsource == 0)
                SystemCoreClock = (8000000UL / 2UL) * pllmull;
            else
                SystemCoreClock = 8000000UL * pllmull;
            break;
        default:
            SystemCoreClock = 8000000UL;
            break;
    }

    /* Apply AHB prescaler */
    tmp = AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> 4];
    SystemCoreClock >>= tmp;
}
