#include "RVA15MD_DataReader.h"
#include "stm32c0xx_hal.h"
#include "main.h"
#include "stm32c0xx_ll_spi.h"
#include "stm32c0xx_ll_gpio.h"

#define CMD_RDID 0x9F
#define CMD_READ 0x03
#define CMD_WREN 0x06
#define CMD_PP   0x02
#define CMD_RDSR 0x05
#define CMD_SE   0xD8
#define STATUS_WIP 0x01

//References for the peripherals used to read the flash
static SPI_TypeDef* const FLASH_SPI = SPI1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

//Status flag. Non-zero when receiving data
static volatile uint8_t isReceivingData = 0;

void readData(uint32_t address24, uint8_t* buffer, uint32_t length)
{
    LL_SPI_Enable(FLASH_SPI);
    LL_GPIO_ResetOutputPin(FLASH_CS_GPIO_Port, FLASH_CS_Pin); //activate chip select
    LL_SPI_TransmitData8(FLASH_SPI, CMD_READ);

    //clock out address
    LL_SPI_TransmitData8(FLASH_SPI, address24 >> 16);

    while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
    LL_SPI_TransmitData8(FLASH_SPI, address24 >> 8);

    while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
    LL_SPI_TransmitData8(FLASH_SPI, address24);

    switch (length)
    {
        default:
            while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
            LL_SPI_ReceiveData8(FLASH_SPI);
            LL_SPI_TransmitData8(FLASH_SPI, 0);
        case 3:
            while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
            LL_SPI_ReceiveData8(FLASH_SPI);
            LL_SPI_TransmitData8(FLASH_SPI, 0);
        case 2:
            while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
            LL_SPI_ReceiveData8(FLASH_SPI);
            LL_SPI_TransmitData8(FLASH_SPI, 0);
        case 1:
            while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
            LL_SPI_ReceiveData8(FLASH_SPI);
            LL_SPI_TransmitData8(FLASH_SPI, 0);
        case 0:
            break;
    }
    switch (length)
    {
        case 1:
            LL_SPI_ReceiveData8(FLASH_SPI);
        case 2:
            LL_SPI_ReceiveData8(FLASH_SPI);
        case 3:
            LL_SPI_ReceiveData8(FLASH_SPI);
        default:
            break;
    }

    uint8_t* const buf_end = buffer + length - 4;

    while ((buf_end - buffer) > 3)
    {
        while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
        LL_SPI_TransmitData8(FLASH_SPI, 0);
        while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
        LL_SPI_TransmitData8(FLASH_SPI, 0);
        while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
        LL_SPI_TransmitData8(FLASH_SPI, 0);
        while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
        LL_SPI_TransmitData8(FLASH_SPI, 0);
    }

    while (buffer < buf_end)
    {
        while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
        LL_SPI_TransmitData8(FLASH_SPI, 0);
    }

    /* Wait until the bus is ready before releasing Chip select */
    while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI)) { }

    LL_GPIO_SetOutputPin(FLASH_CS_GPIO_Port, FLASH_CS_Pin);
    const int rest = length < 4 ? length : 4;
    for (int i = 0; i<rest; i++)
    {
        *buffer++ = LL_SPI_ReceiveData8(FLASH_SPI);
    }
}

void readDataDMA(uint32_t address24, uint8_t* buffer, uint32_t length)
{
    isReceivingData = 1;
    LL_SPI_Enable(FLASH_SPI);
    LL_GPIO_ResetOutputPin(FLASH_CS_GPIO_Port, FLASH_CS_Pin); //activate chip select
    LL_SPI_TransmitData8(FLASH_SPI, CMD_READ);

    //clock out address
    LL_SPI_TransmitData8(FLASH_SPI, address24 >> 16);

    while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
    LL_SPI_TransmitData8(FLASH_SPI, address24 >> 8);

    while(!LL_SPI_IsActiveFlag_TXE(FLASH_SPI)) { }
    LL_SPI_TransmitData8(FLASH_SPI, address24);

    /* Wait until the bus is ready before reading 4 dummy bytes */
    while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI)) { }
    LL_SPI_ReceiveData8(FLASH_SPI);
    LL_SPI_ReceiveData8(FLASH_SPI);
    LL_SPI_ReceiveData8(FLASH_SPI);
    LL_SPI_ReceiveData8(FLASH_SPI);

    /* Reset the threshold bit */
    CLEAR_BIT(FLASH_SPI->CR2, SPI_CR2_LDMATX | SPI_CR2_LDMARX);

    /* Set RX Fifo threshold according the reception data length: 8bit */
    SET_BIT(FLASH_SPI->CR2, SPI_RXFIFO_THRESHOLD);

    /******** RX ****************/
    /* Disable the peripheral */
    __HAL_DMA_DISABLE(&hdma_spi1_rx);

    /* Clear all flags */
    __HAL_DMA_CLEAR_FLAG(&hdma_spi1_rx, (DMA_FLAG_GI1 << (hdma_spi1_rx.ChannelIndex & 0x1cU)));

    /* Configure DMA Channel data length */
    hdma_spi1_rx.Instance->CNDTR = length;

    /* Configure DMA Channel destination address */
    hdma_spi1_rx.Instance->CPAR = LL_SPI_DMA_GetRegAddr(FLASH_SPI);

    /* Configure DMA Channel source address */
    hdma_spi1_rx.Instance->CMAR = (uint32_t)buffer;

    __HAL_DMA_DISABLE_IT(&hdma_spi1_rx, DMA_IT_HT | DMA_IT_TE);
    __HAL_DMA_ENABLE_IT(&hdma_spi1_rx, (DMA_IT_TC));

    /* Enable the Peripheral */
    __HAL_DMA_ENABLE(&hdma_spi1_rx);

    /* Enable Rx DMA Request */
    LL_SPI_EnableDMAReq_RX(FLASH_SPI);

    /******** TX ****************/
    /* Disable the peripheral */
    __HAL_DMA_DISABLE(&hdma_spi1_tx);

    /* Clear all flags */
    __HAL_DMA_CLEAR_FLAG(&hdma_spi1_tx, (DMA_FLAG_GI1 << (hdma_spi1_tx.ChannelIndex & 0x1cU)));

    /* Configure DMA Channel data length */
    hdma_spi1_tx.Instance->CNDTR = length;

    /* Configure DMA Channel destination address */
    hdma_spi1_tx.Instance->CPAR = LL_SPI_DMA_GetRegAddr(FLASH_SPI);

    /* Configure DMA Channel source address */
    hdma_spi1_tx.Instance->CMAR = (uint32_t)buffer;

    /* Enable the Peripheral */
    __HAL_DMA_ENABLE(&hdma_spi1_tx);

    /* Enable Tx DMA Request */
    LL_SPI_EnableDMAReq_TX(FLASH_SPI);
}

void DataReader_DMACallback()
{
    /* Transfer Complete Interrupt management ***********************************/
    if ((0U != (DMA1->ISR & (DMA_FLAG_TC1 << (hdma_spi1_rx.ChannelIndex & 0x1cU)))) && (0U != (hdma_spi1_rx.Instance->CCR & DMA_IT_TC)))
    {
        /* Disable the transfer complete and error interrupt */
        __HAL_DMA_DISABLE_IT(&hdma_spi1_rx, DMA_IT_TE | DMA_IT_TC);
        /* Clear the transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_spi1_rx, (DMA_FLAG_TC1 << (hdma_spi1_rx.ChannelIndex & 0x1cU)));


        // Wait until the bus is not busy before changing configuration
        // SPI is busy in communication or Tx buffer is not empty
        while(LL_SPI_IsActiveFlag_BSY(FLASH_SPI)) { }

        LL_GPIO_SetOutputPin(FLASH_CS_GPIO_Port, FLASH_CS_Pin);

        isReceivingData = 0;
    }
}

void DataReader_ReadData(uint32_t address24, uint8_t* buffer, uint32_t length)
{
    readData(address24, buffer, length);
}

void DataReader_StartDMAReadData(uint32_t address24, uint8_t* buffer, uint32_t length)
{
    readDataDMA(address24, buffer, length);
}

uint32_t DataReader_IsReceivingData(void)
{
    return isReceivingData;
}

void DataReader_WaitForReceiveDone(void)
{
    while (isReceivingData) { }
}
