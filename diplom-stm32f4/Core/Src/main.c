/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart_data.h"
#include "crc8.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
osMessageQId queue_dwin_sendHandle;
osMessageQId queue_data_from_espHandle;
osSemaphoreId sem_rcv_data_from_mqttHandle;
/* USER CODE BEGIN PV */
volatile char rx_data_esp[150];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void StartRcvFromEspTask(void const * argument);
void StartSendToEspTask(void const * argument);
void StartRcvFromDWINTask(void const * argument);
void StartSendToDWINTask(void const * argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */

  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of sem_rcv_data_from_mqtt */
  osSemaphoreDef(sem_rcv_data_from_mqtt);
  sem_rcv_data_from_mqttHandle = osSemaphoreCreate(osSemaphore(sem_rcv_data_from_mqtt), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of queue_dwin_send */
  osMessageQDef(queue_dwin_send, 16, uart_data_t);
  queue_dwin_sendHandle = osMessageCreate(osMessageQ(queue_dwin_send), NULL);

  /* definition and creation of queue_data_from_esp */
  osMessageQDef(queue_data_from_esp, 16, uart_data_t);
  queue_data_from_espHandle = osMessageCreate(osMessageQ(queue_data_from_esp), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    osThreadDef(RcvFromEspDataTask, StartRcvFromEspTask, osPriorityNormal, 0, 1024);
    defaultTaskHandle = osThreadCreate(osThread(RcvFromEspDataTask), NULL);

    osThreadDef(SendToEspDataTask, StartSendToEspTask, osPriorityNormal, 0, 1024);
    defaultTaskHandle = osThreadCreate(osThread(SendToEspDataTask), NULL);

    osThreadDef(RcvFromDWINDataTask, StartRcvFromDWINTask, osPriorityNormal, 0, 1024);
    defaultTaskHandle = osThreadCreate(osThread(RcvFromDWINDataTask), NULL);

    osThreadDef(SendToDWINDataTask, StartSendToDWINTask, osPriorityNormal, 0, 1024);
    defaultTaskHandle = osThreadCreate(osThread(SendToDWINDataTask), NULL);

    HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t*)rx_data_esp,200);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void StartRcvFromEspTask(void const * argument)
{
    /* USER CODE BEGIN 5 */
    dwin_data_t dwin_data;
    uart_data_t uart_data_rcv;
    /* Infinite loop */
    for(;;)
    {
        xQueueReceive(queue_data_from_espHandle, &uart_data_rcv, portMAX_DELAY);

        if (uart_data_rcv.crc == crc8ccitt(&uart_data_rcv, DATA_SIZE)){

            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);

            memset(&dwin_data, (int)'\0',sizeof (dwin_data));
            size_t len_data = strlen(uart_data_rcv.value);

            dwin_data.header = HEADER_PACKET;
            dwin_data.len = len_data + sizeof (dwin_data.address) + sizeof (dwin_data.direction);
            dwin_data.direction = TO_DWIN;
            memcpy(dwin_data.data, uart_data_rcv.value, len_data);

            switch ((packet_type_e)uart_data_rcv.data_type) {
                case DATA_TYPE_STATE:
                    dwin_data.address = ADDR_MESSAGE;
                    //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                    break;
                case DATA_TYPE_DATA:

                    switch ((parametr_name_e)uart_data_rcv.id_parametr) {
                        case PARAMETR_TEMP:
                            dwin_data.address = ADDR_TEMPERATURE;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_HUMIDITY:
                            dwin_data.address = ADDR_HUMIDITY;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_INSOL:
                            dwin_data.address = ADDR_INSOL;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_INPUTS:
                            dwin_data.address = ADDR_INPUT;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_NA:
                        default:
                            break;
                    }
                    break;
                case DATA_TYPE_CMD:
                    break;
                default:
                    break;
            }
        }

        //HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t*)rx_data_esp,200);
    }
    /* USER CODE END 5 */
}

void StartSendToEspTask(void const * argument)
{
    /* USER CODE BEGIN 5 */

    /* Infinite loop */
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    /* USER CODE END 5 */
}

void StartRcvFromDWINTask(void const * argument)
{
    /* USER CODE BEGIN 5 */
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
    /* USER CODE END 5 */
}

void StartSendToDWINTask(void const * argument)
{
    /* USER CODE BEGIN 5 */
    dwin_data_t dwin_data;
    /* Infinite loop */
    for(;;)
    {
        xQueueReceive(queue_dwin_sendHandle, &dwin_data, portMAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)&dwin_data, sizeof (dwin_data_t), 50);
        osDelay(10);
    }
    /* USER CODE END 5 */
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

//    if (huart == &huart2) {
//        HAL_UART_Abort_IT(huart);
//        //uart_data_t data_rcv = *(uart_data_t*)rx_data_esp;
//        portBASE_TYPE xHigherPriorityTaskWoken;
//        //xQueueSendFromISR(queue_data_from_espHandle, &data_rcv, &xHigherPriorityTaskWoken);
//        xSemaphoreGiveFromISR(sem_rcv_data_from_mqttHandle, &xHigherPriorityTaskWoken);
//    }

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart == &huart2) {
        HAL_UART_Abort_IT(huart);
        uart_data_t data_rcv;// = *(uart_data_t*)rx_data_esp;
        size_t size_packet = sizeof (uart_data_t);
        memcpy(&data_rcv, (void*)rx_data_esp, size_packet);
        portBASE_TYPE xHigherPriorityTaskWoken = pdTRUE;
        xQueueSendFromISR(queue_data_from_espHandle, &data_rcv, &xHigherPriorityTaskWoken);
        HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t*)rx_data_esp,200);
    }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
    dwin_data_t dwin_data;
    uart_data_t uart_data_rcv;
    /* Infinite loop */
    for(;;)
    {
        xQueueReceive(queue_data_from_espHandle, &uart_data_rcv, portMAX_DELAY);

        if (uart_data_rcv.crc == crc8ccitt(&uart_data_rcv, DATA_SIZE)){

            memset(&dwin_data, (int)'\0',sizeof (dwin_data));
            size_t len_data = strlen(uart_data_rcv.value);

            dwin_data.header = HEADER_PACKET;
            dwin_data.len = len_data + sizeof (dwin_data.address) + sizeof (dwin_data.direction);
            dwin_data.direction = TO_DWIN;
            memcpy(dwin_data.data, uart_data_rcv.value, len_data);

            switch ((packet_type_e)uart_data_rcv.data_type) {
                case DATA_TYPE_STATE:
                    dwin_data.address = ADDR_MESSAGE;
                    //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                    break;
                case DATA_TYPE_DATA:

                    switch ((parametr_name_e)uart_data_rcv.id_parametr) {
                        case PARAMETR_TEMP:
                            dwin_data.address = ADDR_TEMPERATURE;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_HUMIDITY:
                            dwin_data.address = ADDR_HUMIDITY;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_INSOL:
                            dwin_data.address = ADDR_INSOL;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_INPUTS:
                            dwin_data.address = ADDR_INPUT;
                            //xQueueSend(queue_dwin_sendHandle, &dwin_data, pdMS_TO_TICKS(50));
                            break;
                        case PARAMETR_NA:
                        default:
                            break;
                    }
                    break;
                case DATA_TYPE_CMD:
                    break;
                default:
                    break;
            }
        }

        HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t*)rx_data_esp,200);
    }
  /* USER CODE END 5 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
