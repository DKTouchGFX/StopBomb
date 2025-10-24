#include "stm32c0xx_hal.h"
#include "DCS.h"
#include "main.h"
#include "RVA15MD_DisplayDriver.h"
#include <assert.h>
#include "stm32c0xx_ll_spi.h"
#include "stm32c0xx_ll_gpio.h"

static SPI_TypeDef* const DISPLAY_SPI = SPI2;
extern DMA_HandleTypeDef hdma_spi2_tx;

volatile uint8_t isTransmittingBlock = 0;

static void DisplaySendCommandDataArray(uint8_t command, uint8_t* data, uint32_t size)
{
  LL_GPIO_ResetOutputPin(DISPLAY_CSX_GPIO_Port, DISPLAY_CSX_Pin); //activate display transfer
  LL_GPIO_ResetOutputPin(DISPLAY_DCX_GPIO_Port, DISPLAY_DCX_Pin); //indicate command transfer

  LL_SPI_TransmitData8(DISPLAY_SPI, command);

  // Wait until the bus is not busy before changing configuration
  while(LL_SPI_IsActiveFlag_BSY(DISPLAY_SPI));
  LL_GPIO_SetOutputPin(DISPLAY_DCX_GPIO_Port, DISPLAY_DCX_Pin); //indicate data transfer

  while (size > 0U)
  {
    LL_SPI_TransmitData8(DISPLAY_SPI, *data);
    data++;
    size--;
    /* Wait until TXE flag is set to send data */
    while(!LL_SPI_IsActiveFlag_TXE(DISPLAY_SPI));
  }

  // Wait until the bus is not busy before changing configuration
  while(LL_SPI_IsActiveFlag_BSY(DISPLAY_SPI));
  LL_GPIO_SetOutputPin(DISPLAY_CSX_GPIO_Port, DISPLAY_CSX_Pin);  //de-activate display transfer
}

static void DisplaySendCommand(uint8_t command)
{
  DisplaySendCommandDataArray(command, 0, 0);
}

static void DisplaySendCommandData(uint8_t command, uint8_t data)
{
  DisplaySendCommandDataArray(command, &data, 1);
}

void DisplayDriver_DisplayOn(void)
{
  // Display ON
  DisplaySendCommand(DCS_SET_DISPLAY_ON);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // Turn on LCD backlight
}

void Display_OFF(void)
{
  // Display OFF
  DisplaySendCommand(DCS_SET_DISPLAY_OFF);
  HAL_Delay(10);
}

void Display_Set_Area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t arguments[4];

  // Set columns
  arguments[0] = x0 >> 8;
  arguments[1] = x0 & 0xFF;
  arguments[2] = x1 >> 8;
  arguments[3] = x1 & 0xFF;
  DisplaySendCommandDataArray(DCS_SET_COLUMN_ADDRESS, arguments, 4);

  // Set rows
  arguments[0] = y0 >> 8;
  arguments[1] = y0 & 0xFF;
  arguments[2] = y1 >> 8;
  arguments[3] = y1 & 0xFF;
  DisplaySendCommandDataArray(DCS_SET_PAGE_ADDRESS, arguments, 4);
}

void startDmaTransfer(const uint8_t* data, uint32_t size)
{
  __HAL_DMA_DISABLE(&hdma_spi2_tx);

  // Set the SPI in 16-bit mode to match endianess
  LL_SPI_SetDataWidth(DISPLAY_SPI, LL_SPI_DATAWIDTH_16BIT);

  /* Clear all flags */
  __HAL_DMA_CLEAR_FLAG(&hdma_spi2_tx, (DMA_FLAG_GI1 << (hdma_spi2_tx.ChannelIndex & 0x1cU)));

  /* Configure DMA Channel data length */
  hdma_spi2_tx.Instance->CNDTR = size;

  /* Configure DMA Channel destination address */
  hdma_spi2_tx.Instance->CPAR = LL_SPI_DMA_GetRegAddr(DISPLAY_SPI);

  /* Configure DMA Channel source address */
  hdma_spi2_tx.Instance->CMAR = (uint32_t)data;

  /* Disable the transfer half complete interrupt */
  __HAL_DMA_DISABLE_IT(&hdma_spi2_tx, DMA_IT_HT);
  /* Enable the transfer complete interrupt */
  __HAL_DMA_ENABLE_IT(&hdma_spi2_tx, (DMA_IT_TC | DMA_IT_TE));

  __HAL_DMA_ENABLE(&hdma_spi2_tx);
}

void DisplayDriver_DisplayInit(void)
{
    uint8_t arguments[16];

    DisplaySendCommand(0xFE);
    DisplaySendCommand(0xEF);

    DisplaySendCommandData(0xEB, 0x14);

    DisplaySendCommandData(0x84, 0x60);

    DisplaySendCommandData(0x85, 0xFF); //EN

    DisplaySendCommandData(0x86, 0xFF);

    DisplaySendCommandData(0x87, 0xFF); //EN
    DisplaySendCommandData(0x8E, 0xFF); //EN

    DisplaySendCommandData(0x8F, 0xFF); //EN
    DisplaySendCommandData(0x88, 0x0A); //EN

    DisplaySendCommandData(0x89, 0x23); //spi 2data reg en

    DisplaySendCommandData(0x8A, 0x00);

    DisplaySendCommandData(0x8B, 0x80);

    DisplaySendCommandData(0x8C, 0x01);

    DisplaySendCommandData(0x8D, 0x03); //99 en

    arguments[0] = 0x00;
    arguments[1] = 0x00;  //60:??? 00:????
    DisplaySendCommandDataArray(0xB6, arguments, 2);

    DisplaySendCommandData(0x36, 0x48);

    DisplaySendCommandData(0x3A, 0x05);

    arguments[0] = 0x08;
    arguments[1] = 0x08;
    arguments[2] = 0x08;
    arguments[3] = 0x08;
    DisplaySendCommandDataArray(0x90, arguments, 4);

    DisplaySendCommandData(0xBA, 0x0A); //TE width
    DisplaySendCommandData(0xBD, 0x06);

////cp
//    SPI_WriteComm(0xA6);
//    SPI_WriteData(0x74);

//    SPI_WriteComm(0xBF);
//    SPI_WriteData(0x1C);

//    SPI_WriteComm(0xA7);
//    SPI_WriteData(0x45);

//    SPI_WriteComm(0xA9);
//    SPI_WriteData(0xBB);
//
//    SPI_WriteComm(0xB8);
//    SPI_WriteData(0x63);

    DisplaySendCommandData(0xBC, 0x00);

    arguments[0] = 0x60;
    arguments[1] = 0x01;
    arguments[2] = 0x04;
    DisplaySendCommandDataArray(0xFF, arguments, 3);

//    SPI_WriteComm(0xC0);
//    SPI_WriteData(0x0E);  //test

    DisplaySendCommandData(0xC3, 0x18); //18

    DisplaySendCommandData(0xC4, 0x18); //18

    DisplaySendCommandData(0xC9, 0x3F);

    DisplaySendCommandData(0xBE, 0x11);

    arguments[0] = 0x10;
    arguments[1] = 0x0E;
    DisplaySendCommandDataArray(0xE1, arguments, 2);

    arguments[0] = 0x21;
    arguments[1] = 0x0c; ///source 1t  10
    arguments[2] = 0x02;
    DisplaySendCommandDataArray(0xDF, arguments, 3);

    arguments[0] = 0x4C;
    arguments[1] = 0x10;
    arguments[2] = 0x09;
    arguments[3] = 0x09;
    arguments[4] = 0x86;
    arguments[5] = 0x32;
    DisplaySendCommandDataArray(0xF0, arguments, 6);

    arguments[0] = 0x48;
    arguments[1] = 0x75;
    arguments[2] = 0x95;
    arguments[3] = 0x2E;
    arguments[4] = 0x34;
    arguments[5] = 0x8F;
    DisplaySendCommandDataArray(0xF1, arguments, 6);

    arguments[0] = 0x4C;
    arguments[1] = 0x10;
    arguments[2] = 0x09;
    arguments[3] = 0x09;
    arguments[4] = 0x86;
    arguments[5] = 0x32;
    DisplaySendCommandDataArray(0xF2, arguments, 6);

    arguments[0] = 0x48;
    arguments[1] = 0x75;
    arguments[2] = 0x95;
    arguments[3] = 0x2E;
    arguments[4] = 0x34;
    arguments[5] = 0x8F;
    DisplaySendCommandDataArray(0xF3, arguments, 6);

    arguments[0] = 0x1B;
    arguments[1] = 0x0B;
    DisplaySendCommandDataArray(0xED, arguments, 2);

//    SPI_WriteComm(0xAC);//cp
//    SPI_WriteData(0x47);

    DisplaySendCommandData(0xAE, 0x77);

//    SPI_WriteComm(0xCB);
//    SPI_WriteData(0x02);

    DisplaySendCommandData(0xCD, 0x63);

    arguments[0] = 0x07;
    arguments[1] = 0x07;
    arguments[2] = 0x04;
    arguments[3] = 0x0E;
    arguments[4] = 0x0F;
    arguments[5] = 0x09;
    arguments[6] = 0x07;
    arguments[7] = 0x08;
    arguments[8] = 0x03;
    DisplaySendCommandDataArray(0x70, arguments, 9);

    DisplaySendCommandData(0xE8, 0x34); // 04:column 14:1-dot 24:2-dot inversion

//SPI_WriteComm(0xE9);
//SPI_WriteData(0x08);///spi 2 data
    arguments[0] = 0x18;
    arguments[1] = 0x0D;
    arguments[2] = 0x71;
    arguments[3] = 0xED;
    arguments[4] = 0x70;
    arguments[5] = 0x70;
    arguments[6] = 0x18;
    arguments[7] = 0x0F;
    arguments[8] = 0x71;
    arguments[9] = 0xEF;
    arguments[10] = 0x70;
    arguments[11] = 0x70;
    DisplaySendCommandDataArray(0x62, arguments, 12);

    arguments[0] = 0x18;
    arguments[1] = 0x11;
    arguments[2] = 0x71;
    arguments[3] = 0xF1;
    arguments[4] = 0x70;
    arguments[5] = 0x70;
    arguments[6] = 0x18;
    arguments[7] = 0x13;
    arguments[8] = 0x71;
    arguments[9] = 0xF3;
    arguments[10] = 0x70;
    arguments[11] = 0x70;
    DisplaySendCommandDataArray(0x63, arguments, 12);

    arguments[0] = 0x3B; ///hsd
    arguments[1] = 0x29;
    arguments[2] = 0xF1;
    arguments[3] = 0x01;
    arguments[4] = 0xF1;
    arguments[5] = 0x00;
    arguments[6] = 0x0A; ///hsd
    DisplaySendCommandDataArray(0x64, arguments, 7);

    arguments[0] = 0x3C;
    arguments[1] = 0x00;
    arguments[2] = 0xCD;
    arguments[3] = 0x67;
    arguments[4] = 0x45;
    arguments[5] = 0x45;
    arguments[6] = 0x10;
    arguments[7] = 0x00;
    arguments[8] = 0x00;
    arguments[9] = 0x00;
    DisplaySendCommandDataArray(0x66, arguments, 10);

    arguments[0] = 0x00;
    arguments[1] = 0x3C;
    arguments[2] = 0x00;
    arguments[3] = 0x00;
    arguments[4] = 0x00;
    arguments[5] = 0x01;
    arguments[6] = 0x54;
    arguments[7] = 0x10;
    arguments[8] = 0x32;
    arguments[9] = 0x98;
    DisplaySendCommandDataArray(0x67, arguments, 10);

    DisplaySendCommandData(0x74, 0x10); // related to refresh rate

    arguments[0] = 0x3e;
    arguments[1] = 0x07;
    DisplaySendCommandDataArray(0x98, arguments, 2);

    arguments[0] = 0x3e;
    arguments[1] = 0x07;
    DisplaySendCommandDataArray(0x99, arguments, 2); //bvee 2x

    DisplaySendCommand(0x35);
    DisplaySendCommand(0x21);

    DisplaySendCommand(0x11);
    HAL_Delay(120);
}

void DisplayDriver_DisplayReset(void)
{
  LL_GPIO_ResetOutputPin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin);
  HAL_Delay(50);
  LL_GPIO_SetOutputPin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin);
  HAL_Delay(100);
}

void DisplayDriver_Init(void)
{
  LL_SPI_Enable(DISPLAY_SPI);
  LL_SPI_EnableDMAReq_TX(DISPLAY_SPI);

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

int touchgfxDisplayDriverTransmitActive(void)
{
  return isTransmittingBlock;
}

void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  isTransmittingBlock = 1;

  Display_Set_Area(x, y, x + w - 1, y + h - 1);

  LL_GPIO_ResetOutputPin(DISPLAY_CSX_GPIO_Port, DISPLAY_CSX_Pin); //activate display transfer
  LL_GPIO_ResetOutputPin(DISPLAY_DCX_GPIO_Port, DISPLAY_DCX_Pin); //indicate command transfer

  LL_SPI_TransmitData8(DISPLAY_SPI, DCS_WRITE_MEMORY_START);
  // Wait until the bus is not busy before changing configuration
  while(LL_SPI_IsActiveFlag_BSY(DISPLAY_SPI));
  LL_GPIO_SetOutputPin(DISPLAY_DCX_GPIO_Port, DISPLAY_DCX_Pin); //indicate data transfer

  startDmaTransfer(pixels, w * h);
}

void DisplayDriver_DMACallback(void)
{
  /* Transfer Complete Interrupt management ***********************************/
  if ((0U != (DMA1->ISR & (DMA_FLAG_TC1))) && (0U != (hdma_spi2_tx.Instance->CCR & DMA_IT_TC)))
  {
    /* Disable the transfer complete and error interrupt */
    __HAL_DMA_DISABLE_IT(&hdma_spi2_tx, DMA_IT_TE | DMA_IT_TC);

    /* Clear the transfer complete flag */
    __HAL_DMA_CLEAR_FLAG(&hdma_spi2_tx, DMA_FLAG_TC1);

    isTransmittingBlock = 0;

    // Wait until the bus is not busy before changing configuration
    // SPI is busy in communication or Tx buffer is not empty
    while(LL_SPI_IsActiveFlag_BSY(DISPLAY_SPI));

    LL_GPIO_SetOutputPin(DISPLAY_CSX_GPIO_Port, DISPLAY_CSX_Pin);  //de-activate display transfer

    // Go back to 8-bit mode
    LL_SPI_SetDataWidth(DISPLAY_SPI, LL_SPI_DATAWIDTH_8BIT);

    // Signal Transfer Complete to TouchGFX
    DisplayDriver_TransferCompleteCallback();
  }
    /* Transfer Error Interrupt management **************************************/
  else if ((0U != (DMA1->ISR & (DMA_FLAG_TC1))) && (0U != (hdma_spi2_tx.Instance->CCR & DMA_IT_TE)))
  {
    /* When a DMA transfer error occurs */
    /* A hardware clear of its EN bits is performed */
    /* Disable ALL DMA IT */
    __HAL_DMA_DISABLE_IT(&hdma_spi2_tx, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));

    /* Clear all flags */
    __HAL_DMA_CLEAR_FLAG(&hdma_spi2_tx, DMA_FLAG_GI1 );

    assert(0);  // Halting program - Transfer Error Interrupt received.
  }
}
