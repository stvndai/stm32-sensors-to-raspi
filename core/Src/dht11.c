#include "dht11.h"

// ── DWT cycle counter for precise microsecond delays ──────────────────────────

static void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetSysClockFreq() / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

// ── GPIO helpers ──────────────────────────────────────────────────────────────

void DHT11_SetPinOutput(GPIO_TypeDef *GPIOx, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11_SetPinInput(GPIO_TypeDef *GPIOx, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// ── Internal helper — wait for pin state with timeout ────────────────────────

static uint8_t wait_for_pin(GPIO_PinState state, uint32_t timeout_us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = timeout_us * (HAL_RCC_GetSysClockFreq() / 1000000);
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state) {
        if ((DWT->CYCCNT - start) > ticks) return 0;
    }
    return 1;
}

// ── Public API ────────────────────────────────────────────────────────────────

void DHT11_Init(void) {
    DWT_Init();
}

uint8_t DHT11_Read(DHT11_Data *data) {
    uint8_t bits[40] = {0};
    uint8_t bytes[5] = {0};

    // START SIGNAL
    DHT11_SetPinOutput(DHT11_PORT, DHT11_PIN);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);

    // WAIT FOR SENSOR RESPONSE
    DHT11_SetPinInput(DHT11_PORT, DHT11_PIN);

    if (!wait_for_pin(GPIO_PIN_RESET, 100)) return 1;
    if (!wait_for_pin(GPIO_PIN_SET,   100)) return 2;
    if (!wait_for_pin(GPIO_PIN_RESET, 100)) return 3;

    // READ 40 BITS
    for (int i = 0; i < 40; i++) {
        if (!wait_for_pin(GPIO_PIN_SET,   100)) return 4;
        delay_us(40);
        bits[i] = (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) ? 1 : 0;
        if (!wait_for_pin(GPIO_PIN_RESET, 100)) return 5;
    }

    // ASSEMBLE BYTES
    for (int i = 0; i < 40; i++) {
        bytes[i / 8] <<= 1;
        bytes[i / 8] |= bits[i];
    }

    // VERIFY CHECKSUM
    if (bytes[4] != ((bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF))
        return 6;

    data->humidity_int = bytes[0];
    data->humidity_dec = bytes[1];
    data->temp_int     = bytes[2];
    data->temp_dec     = bytes[3];
    data->checksum     = bytes[4];

    return 1;
}
