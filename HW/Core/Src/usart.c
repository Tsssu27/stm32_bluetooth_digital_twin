/**
  * @file    usart.c
  * @brief   USART1 and printf redirection, plus Rx command parsing.
  */

#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "encoder.h" // For Encoder_SetAngle

UART_HandleTypeDef huart1;

/* Rx buffer for receiving Bluetooth string */
static uint8_t rx_byte;
#define RX_BUF_SIZE 32
static char rx_buf[RX_BUF_SIZE];
static uint8_t rx_idx = 0;

/**
 * @brief Initialize USART1 (9600, 8N1)
 */
void USART1_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        /* Init Error */
    }

    /* Start receiving the first byte via Interrupt */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/**
 * @brief UART MSP Init (called by HAL_UART_Init)
 */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance == USART1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10    ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
 * @brief UART Rx Complete Callback (Interrupt Context)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rx_byte == '\n' || rx_byte == '\r')
        {
            /* Null-terminate the string */
            rx_buf[rx_idx] = '\0';
            
            if (rx_idx > 0)
            {
                /* Expected format: <CMD:ANGLE:xxx> */
                int angle_val;
                if (sscanf(rx_buf, "<CMD:ANGLE:%d>", &angle_val) == 1)
                {
                    /* Limit angle to valid range 0~180 */
                    if (angle_val < 0) angle_val = 0;
                    if (angle_val > 180) angle_val = 180;
                    
                    /* Programmatically change the encoder value */
                    Encoder_SetAngle((uint16_t)angle_val);
                }
            }
            
            /* Reset buffer */
            rx_idx = 0;
        }
        else
        {
            if (rx_idx < (RX_BUF_SIZE - 1))
            {
                rx_buf[rx_idx++] = (char)rx_byte;
            }
            else
            {
                /* Overflow protection */
                rx_idx = 0;
            }
        }
        
        /* Re-enable Rx Interrupt */
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

/**
 * @brief Override standard _write to redirect printf to UART
 */
int _write(int file, char *ptr, int len)
{
    /* To avoid compiler warning */
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
