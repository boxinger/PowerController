#include "oledll.h"

#include <string.h>

#include "i2c.h"

#define OLEDLL_CTRL_CMD          0x00U
#define OLEDLL_CTRL_DATA         0x40U
#define OLEDLL_INIT_DELAY_MS     100U
#define OLEDLL_PAGE_COUNT        (OLEDLL_HEIGHT / 8U)
#define OLEDLL_TX_PREFIX_SIZE    8U
#define OLEDLL_TX_BUFFER_SIZE    (OLEDLL_TX_PREFIX_SIZE + OLEDLL_BUFFER_SIZE)

static uint8_t frameBuffer[OLEDLL_BUFFER_SIZE] = {0};
static uint8_t txBuffer[OLEDLL_TX_BUFFER_SIZE] = {0};

static volatile uint8_t OLEDLL_Initialized = 0U;
static volatile uint8_t OLEDLL_IsTransmitting = 0U;
static volatile uint8_t OLEDLL_IsDirty = 0U;
static volatile uint8_t OLEDLL_IsBusy = 0U;

static void OLEDLL_PrepareTxPrefix(void)
{
	txBuffer[0] = OLEDLL_CTRL_CMD;
	txBuffer[1] = 0x21U;
	txBuffer[2] = 0x00U;
	txBuffer[3] = (uint8_t)(OLEDLL_WIDTH - 1U);
	txBuffer[4] = 0x22U;
	txBuffer[5] = 0x00U;
	txBuffer[6] = (uint8_t)(OLEDLL_PAGE_COUNT - 1U);
	txBuffer[7] = OLEDLL_CTRL_DATA;
}

static OLEDLL_StatusTypeDef OLEDLL_WriteCommand(uint8_t command)
{
	if (HAL_I2C_Mem_Write(&OLEDLL_HANDLE,
						  OLEDLL_ADDR,
						  OLEDLL_CTRL_CMD,
						  I2C_MEMADD_SIZE_8BIT,
						  &command,
						  1U,
						  OLEDLL_TIMEOUT) != HAL_OK) {
		return OLEDLL_ERROR;
	}

	return OLEDLL_OK;
}

static OLEDLL_StatusTypeDef OLEDLL_WriteCommandSequence(const uint8_t* commands, uint16_t length)
{
	uint16_t index;

	if (commands == NULL) {
		return OLEDLL_INVALID_PARAM;
	}

	for (index = 0U; index < length; index++) {
		if (OLEDLL_WriteCommand(commands[index]) != OLEDLL_OK) {
			return OLEDLL_ERROR;
		}
	}

	return OLEDLL_OK;
}

OLEDLL_StatusTypeDef OLEDLL_Init(void)
{
	static const uint8_t initSequence[] = {
		0xAEU,
		0x20U, 0x00U,
		0xB0U,
		0xC8U,
		0x00U,
		0x10U,
		0x40U,
		0x81U, 0xCFU,
		0xA1U,
		0xA6U,
		0xA8U, 0x3FU,
		0xA4U,
		0xD3U, 0x00U,
		0xD5U, 0x80U,
		0xD9U, 0xF1U,
		0xDAU, 0x12U,
		0xDBU, 0x40U,
		0x8DU, 0x14U
	};

	HAL_Delay(OLEDLL_INIT_DELAY_MS);

	memset(frameBuffer, 0, sizeof(frameBuffer));
	memset(txBuffer, 0, sizeof(txBuffer));
	OLEDLL_PrepareTxPrefix();
	OLEDLL_IsTransmitting = 0U;
	OLEDLL_IsBusy = 0U;
	OLEDLL_IsDirty = 0U;
	OLEDLL_Initialized = 0U;

	if (OLEDLL_WriteCommandSequence(initSequence, (uint16_t)sizeof(initSequence)) != OLEDLL_OK) {
		return OLEDLL_ERROR;
	}

	if (OLEDLL_WriteCommand(0xAFU) != OLEDLL_OK) {
		return OLEDLL_ERROR;
	}

	OLEDLL_Initialized = 1U;
	return OLEDLL_OK;
}

OLEDLL_StatusTypeDef OLEDLL_Clear(void)
{
	memset(frameBuffer, 0, sizeof(frameBuffer));
	OLEDLL_IsDirty = 1U;
	return OLEDLL_OK;
}

OLEDLL_StatusTypeDef OLEDLL_WriteFrame(const uint8_t* buffer)
{
	if (buffer == NULL) {
		return OLEDLL_INVALID_PARAM;
	}

    

	memcpy(frameBuffer, buffer, sizeof(frameBuffer));
	OLEDLL_IsDirty = 1U;
	return OLEDLL_OK;
}

OLEDLL_StatusTypeDef OLEDLL_Update(void)
{
	HAL_StatusTypeDef halStatus;

	if (OLEDLL_Initialized == 0U) {
		return OLEDLL_ERROR;
	}

	if (OLEDLL_IsTransmitting != 0U) {
		return OLEDLL_BUSY;
	}

	if (OLEDLL_IsDirty == 0U) {
		return OLEDLL_OK;
	}

	OLEDLL_IsTransmitting = 1U;
	OLEDLL_IsDirty = 0U;

	memcpy(&txBuffer[OLEDLL_TX_PREFIX_SIZE], frameBuffer, sizeof(frameBuffer));

	halStatus = HAL_I2C_Master_Transmit_DMA(&OLEDLL_HANDLE,
											OLEDLL_ADDR,
											txBuffer,
											(uint16_t)sizeof(txBuffer));
	if (halStatus != HAL_OK) {
		OLEDLL_IsTransmitting = 0U;
		OLEDLL_IsDirty = 1U;
		return OLEDLL_ERROR;
	}

	return OLEDLL_OK;
}

void OLEDLL_TxCpltCallback(void)
{
	OLEDLL_IsTransmitting = 0U;
}

void OLEDLL_ErrorCallback(void)
{
	OLEDLL_IsTransmitting = 0U;
	OLEDLL_IsDirty = 1U;
}



