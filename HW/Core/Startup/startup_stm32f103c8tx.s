/**
  * @file    startup_stm32f103c8tx.s
  * @brief   STM32F103C8T6 startup file (ARM GCC syntax)
  *
  * Vector table + Reset_Handler that copies .data and zeros .bss,
  * then calls SystemInit and main().
  */

  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/**
  * @brief  Reset_Handler
  */
  .section .text.Reset_Handler,"ax",%progbits
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr   r0, =_estack
  mov   sp, r0

/* Copy .data section from Flash to RAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b     LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

/* Zero-fill .bss section */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Call SystemInit, then main */
  bl  SystemInit
  bl  main
  bx  lr

.size Reset_Handler, .-Reset_Handler

/**
  * @brief  Default_Handler — catches unimplemented IRQs
  */
  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler

/**
  * @brief  Vector Table (placed at 0x08000000)
  */
  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object

g_pfnVectors:
  .word _estack                     /* Top of Stack                    */
  .word Reset_Handler               /* Reset Handler                   */
  .word NMI_Handler                 /* NMI Handler                     */
  .word HardFault_Handler           /* Hard Fault Handler              */
  .word MemManage_Handler           /* MPU Fault Handler               */
  .word BusFault_Handler            /* Bus Fault Handler               */
  .word UsageFault_Handler          /* Usage Fault Handler             */
  .word 0                           /* Reserved                        */
  .word 0                           /* Reserved                        */
  .word 0                           /* Reserved                        */
  .word 0                           /* Reserved                        */
  .word SVC_Handler                 /* SVCall Handler                  */
  .word DebugMon_Handler            /* Debug Monitor Handler           */
  .word 0                           /* Reserved                        */
  .word PendSV_Handler              /* PendSV Handler                  */
  .word SysTick_Handler             /* SysTick Handler                 */
  /* External Interrupts */
  .word WWDG_IRQHandler             /* 0:  Window Watchdog             */
  .word PVD_IRQHandler              /* 1:  PVD through EXTI            */
  .word TAMPER_IRQHandler           /* 2:  Tamper                      */
  .word RTC_IRQHandler              /* 3:  RTC                         */
  .word FLASH_IRQHandler            /* 4:  Flash                       */
  .word RCC_IRQHandler              /* 5:  RCC                         */
  .word EXTI0_IRQHandler            /* 6:  EXTI Line 0  (CLK -> PB0)  */
  .word EXTI1_IRQHandler            /* 7:  EXTI Line 1                 */
  .word EXTI2_IRQHandler            /* 8:  EXTI Line 2                 */
  .word EXTI3_IRQHandler            /* 9:  EXTI Line 3                 */
  .word EXTI4_IRQHandler            /* 10: EXTI Line 4                 */
  .word DMA1_Channel1_IRQHandler    /* 11: DMA1 Channel 1              */
  .word DMA1_Channel2_IRQHandler    /* 12: DMA1 Channel 2              */
  .word DMA1_Channel3_IRQHandler    /* 13: DMA1 Channel 3              */
  .word DMA1_Channel4_IRQHandler    /* 14: DMA1 Channel 4              */
  .word DMA1_Channel5_IRQHandler    /* 15: DMA1 Channel 5              */
  .word DMA1_Channel6_IRQHandler    /* 16: DMA1 Channel 6              */
  .word DMA1_Channel7_IRQHandler    /* 17: DMA1 Channel 7              */
  .word ADC1_2_IRQHandler           /* 18: ADC1 & ADC2                 */
  .word USB_HP_CAN1_TX_IRQHandler   /* 19: USB HP / CAN1 TX            */
  .word USB_LP_CAN1_RX0_IRQHandler  /* 20: USB LP / CAN1 RX0           */
  .word CAN1_RX1_IRQHandler         /* 21: CAN1 RX1                    */
  .word CAN1_SCE_IRQHandler         /* 22: CAN1 SCE                    */
  .word EXTI9_5_IRQHandler          /* 23: EXTI Lines 5-9 (SW -> PB5) */
  .word TIM1_BRK_IRQHandler         /* 24: TIM1 Break                  */
  .word TIM1_UP_IRQHandler          /* 25: TIM1 Update                 */
  .word TIM1_TRG_COM_IRQHandler     /* 26: TIM1 Trigger / Commutation  */
  .word TIM1_CC_IRQHandler          /* 27: TIM1 Capture Compare        */
  .word TIM2_IRQHandler             /* 28: TIM2 (PWM servo)            */
  .word TIM3_IRQHandler             /* 29: TIM3                        */
  .word TIM4_IRQHandler             /* 30: TIM4                        */
  .word I2C1_EV_IRQHandler          /* 31: I2C1 Event (OLED)           */
  .word I2C1_ER_IRQHandler          /* 32: I2C1 Error                  */
  .word I2C2_EV_IRQHandler          /* 33: I2C2 Event                  */
  .word I2C2_ER_IRQHandler          /* 34: I2C2 Error                  */
  .word SPI1_IRQHandler             /* 35: SPI1                        */
  .word SPI2_IRQHandler             /* 36: SPI2                        */
  .word USART1_IRQHandler           /* 37: USART1                      */
  .word USART2_IRQHandler           /* 38: USART2                      */
  .word USART3_IRQHandler           /* 39: USART3                      */
  .word EXTI15_10_IRQHandler        /* 40: EXTI Lines 10-15            */
  .word RTC_Alarm_IRQHandler        /* 41: RTC Alarm via EXTI          */
  .word USBWakeUp_IRQHandler        /* 42: USB Wakeup via EXTI         */

  .size g_pfnVectors, .-g_pfnVectors

/*
 * Weak aliases — any module can override these by defining the same symbol
 */
  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler
  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler
  .weak MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler
  .weak BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler
  .weak UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler
  .weak SVC_Handler
  .thumb_set SVC_Handler,Default_Handler
  .weak DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler
  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler
  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler
  .weak WWDG_IRQHandler
  .thumb_set WWDG_IRQHandler,Default_Handler
  .weak PVD_IRQHandler
  .thumb_set PVD_IRQHandler,Default_Handler
  .weak TAMPER_IRQHandler
  .thumb_set TAMPER_IRQHandler,Default_Handler
  .weak RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_Handler
  .weak FLASH_IRQHandler
  .thumb_set FLASH_IRQHandler,Default_Handler
  .weak RCC_IRQHandler
  .thumb_set RCC_IRQHandler,Default_Handler
  .weak EXTI0_IRQHandler
  .thumb_set EXTI0_IRQHandler,Default_Handler
  .weak EXTI1_IRQHandler
  .thumb_set EXTI1_IRQHandler,Default_Handler
  .weak EXTI2_IRQHandler
  .thumb_set EXTI2_IRQHandler,Default_Handler
  .weak EXTI3_IRQHandler
  .thumb_set EXTI3_IRQHandler,Default_Handler
  .weak EXTI4_IRQHandler
  .thumb_set EXTI4_IRQHandler,Default_Handler
  .weak DMA1_Channel1_IRQHandler
  .thumb_set DMA1_Channel1_IRQHandler,Default_Handler
  .weak DMA1_Channel2_IRQHandler
  .thumb_set DMA1_Channel2_IRQHandler,Default_Handler
  .weak DMA1_Channel3_IRQHandler
  .thumb_set DMA1_Channel3_IRQHandler,Default_Handler
  .weak DMA1_Channel4_IRQHandler
  .thumb_set DMA1_Channel4_IRQHandler,Default_Handler
  .weak DMA1_Channel5_IRQHandler
  .thumb_set DMA1_Channel5_IRQHandler,Default_Handler
  .weak DMA1_Channel6_IRQHandler
  .thumb_set DMA1_Channel6_IRQHandler,Default_Handler
  .weak DMA1_Channel7_IRQHandler
  .thumb_set DMA1_Channel7_IRQHandler,Default_Handler
  .weak ADC1_2_IRQHandler
  .thumb_set ADC1_2_IRQHandler,Default_Handler
  .weak USB_HP_CAN1_TX_IRQHandler
  .thumb_set USB_HP_CAN1_TX_IRQHandler,Default_Handler
  .weak USB_LP_CAN1_RX0_IRQHandler
  .thumb_set USB_LP_CAN1_RX0_IRQHandler,Default_Handler
  .weak CAN1_RX1_IRQHandler
  .thumb_set CAN1_RX1_IRQHandler,Default_Handler
  .weak CAN1_SCE_IRQHandler
  .thumb_set CAN1_SCE_IRQHandler,Default_Handler
  .weak EXTI9_5_IRQHandler
  .thumb_set EXTI9_5_IRQHandler,Default_Handler
  .weak TIM1_BRK_IRQHandler
  .thumb_set TIM1_BRK_IRQHandler,Default_Handler
  .weak TIM1_UP_IRQHandler
  .thumb_set TIM1_UP_IRQHandler,Default_Handler
  .weak TIM1_TRG_COM_IRQHandler
  .thumb_set TIM1_TRG_COM_IRQHandler,Default_Handler
  .weak TIM1_CC_IRQHandler
  .thumb_set TIM1_CC_IRQHandler,Default_Handler
  .weak TIM2_IRQHandler
  .thumb_set TIM2_IRQHandler,Default_Handler
  .weak TIM3_IRQHandler
  .thumb_set TIM3_IRQHandler,Default_Handler
  .weak TIM4_IRQHandler
  .thumb_set TIM4_IRQHandler,Default_Handler
  .weak I2C1_EV_IRQHandler
  .thumb_set I2C1_EV_IRQHandler,Default_Handler
  .weak I2C1_ER_IRQHandler
  .thumb_set I2C1_ER_IRQHandler,Default_Handler
  .weak I2C2_EV_IRQHandler
  .thumb_set I2C2_EV_IRQHandler,Default_Handler
  .weak I2C2_ER_IRQHandler
  .thumb_set I2C2_ER_IRQHandler,Default_Handler
  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler
  .weak SPI2_IRQHandler
  .thumb_set SPI2_IRQHandler,Default_Handler
  .weak USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_Handler
  .weak USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Default_Handler
  .weak USART3_IRQHandler
  .thumb_set USART3_IRQHandler,Default_Handler
  .weak EXTI15_10_IRQHandler
  .thumb_set EXTI15_10_IRQHandler,Default_Handler
  .weak RTC_Alarm_IRQHandler
  .thumb_set RTC_Alarm_IRQHandler,Default_Handler
  .weak USBWakeUp_IRQHandler
  .thumb_set USBWakeUp_IRQHandler,Default_Handler
