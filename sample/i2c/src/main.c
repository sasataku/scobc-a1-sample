/*
 * Copyright (c) 2025 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/i2c.h>

#define TMP175_I2C_ADDR (0x4F)
#define TMP175_TEMP_REG (0x00)

int main(void)
{
	int ret;
	uint8_t data[2];
	float temp;

	const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));

	ret = i2c_burst_read(i2c, TMP175_I2C_ADDR, TMP175_TEMP_REG, data, ARRAY_SIZE(data));
	if (ret < 0) {
		printf("Failed to read from Temperature Sensor (%d)\n", ret);
		goto end;
	}

	data[1] = data[1] >> 4;
	temp = (int8_t)data[0] + (float)data[1] * 0.0625f;

	printf("Temperature: %.4f [deg]\n", (double)temp);

end:
	return ret;
}
