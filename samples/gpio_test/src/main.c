/*
 * Copyright (c) 2025 Space Cubics Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell_uart.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/drivers/gpio.h>

#define UIO1 (1U)
#define UIO2 (2U)

static const struct device *get_device(uint8_t no)
{
	const struct device *dev;

	switch (no) {
	case 1:
		dev = DEVICE_DT_GET(DT_NODELABEL(uio1));
		break;
	case 2:
		dev = DEVICE_DT_GET(DT_NODELABEL(uio2));
		break;
	default:
		return NULL;
	}

	if (!device_is_ready(dev)) {
		return NULL;
	}

	return dev;
}

static int gpio_read(uint8_t uio, uint8_t port, uint8_t *status)
{
	int ret;
	const struct device *dev;
	uint32_t val;

	dev = get_device(uio);
	if (dev == NULL) {
		ret = -EINVAL;
		goto end;
	}

	ret = gpio_port_get(dev, &val);
	if (ret < 0) {
		goto end;
	}

	*status = (val & BIT(port)) >> port;

end:
	return ret;
}

static int cmd_gpio_read(const struct shell *sh, size_t argc, char **argv)
{
	int ret;
	uint8_t uio;
	uint8_t port;
	char *endp;
	uint8_t status;

	if (argc < 3) {
		shell_error(sh, "Usage: gpio read <uio> <port>");
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	uio = strtol(argv[1], &endp, 10);
	if ((endp == argv[1]) || (*endp != '\0')) {
		shell_error(sh, "Invalid uio: %s", argv[1]);
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	port = strtol(argv[2], &endp, 10);
	if ((endp == argv[2]) || (*endp != '\0')) {
		shell_error(sh, "Invalid port: %s", argv[2]);
		ret = -EINVAL;
		goto end;
	}

	ret = gpio_read(uio, port, &status);
	if (ret < 0) {
		shell_error(sh, "Read status failed. UIO%d, Port:%d", uio, port);
		goto end;
	}

	shell_info(sh, "Read UIO%d Port:%d status: %d", uio, port, status);

end:
	return ret;
}

static int gpio_write(uint8_t uio, uint8_t port, uint8_t status)
{
	int ret;
	const struct device *dev;

	dev = get_device(uio);
	if (dev == NULL) {
		ret = -EINVAL;
		goto end;
	}

	ret = gpio_port_set_masked_raw(dev, BIT(port), status << port);

end:
	return ret;
}

static int cmd_gpio_write(const struct shell *sh, size_t argc, char **argv)
{
	int ret;
	uint8_t uio;
	uint8_t port;
	uint8_t status;
	char *endp;

	if (argc < 4) {
		shell_error(sh, "Usage: gpio write <uio> <port> <status>");
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	uio = strtol(argv[1], &endp, 10);
	if ((endp == argv[1]) || (*endp != '\0')) {
		shell_error(sh, "Invalid uio: %s", argv[1]);
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	port = strtol(argv[2], &endp, 10);
	if ((endp == argv[2]) || (*endp != '\0')) {
		shell_error(sh, "Invalid port: %s", argv[2]);
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	status = strtol(argv[3], &endp, 10);
	if ((endp == argv[3]) || (*endp != '\0')) {
		shell_error(sh, "Invalid port: %s", argv[3]);
		ret = -EINVAL;
		goto end;
	}

	ret = gpio_write(uio, port, status);
	if (ret < 0) {
		shell_error(sh, "Write status failed. UIO%d, Port:%d Status:%d (%d)", uio, port, status, ret);
		goto end;
	}

	shell_info(sh, "Write UIO%d Port:%d Status : %d", uio, port, status);

end:
	return ret;
}

static int gpio_direction(uint8_t uio, uint8_t port, uint8_t direction)
{
	int ret;
	const struct device *dev;
	gpio_flags_t flags;

	dev = get_device(uio);
	if (dev == NULL) {
		ret = -EINVAL;
		goto end;
	}

	if (direction == 0) {
		flags = GPIO_INPUT;
	} else {
		flags = GPIO_OUTPUT;
	}

	ret = gpio_pin_configure(dev, port, flags);

end:
	return ret;
}

static int cmd_gpio_direction(const struct shell *sh, size_t argc, char **argv)
{
	int ret;
	uint8_t uio;
	uint8_t port;
	uint8_t direction;
	char *endp;

	if (argc < 4) {
		shell_error(sh, "Usage: gpio direction <uio> <port> <direction>");
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	uio = strtol(argv[1], &endp, 10);
	if ((endp == argv[1]) || (*endp != '\0')) {
		shell_error(sh, "Invalid uio: %s", argv[1]);
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	port = strtol(argv[2], &endp, 10);
	if ((endp == argv[2]) || (*endp != '\0')) {
		shell_error(sh, "Invalid port: %s", argv[2]);
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	direction = strtol(argv[3], &endp, 10);
	if ((endp == argv[3]) || (*endp != '\0')) {
		shell_error(sh, "Invalid direction: %s", argv[3]);
		ret = -EINVAL;
		goto end;
	}

	ret = gpio_direction(uio, port, direction);
	if (ret < 0) {
		shell_error(sh, "Change direction failed. UIO%d, Port:%d Direction:%d (%d)", uio, port, direction, ret);
		goto end;
	}

	shell_info(sh, "Change direction UIO%d Port:%d direction: %d", uio, port, direction);

end:
	return ret;
}

static int gpio_change_direction_to_input(const struct shell *sh, uint8_t pin)
{
	int ret;
	uint8_t target = 0;

	ret = gpio_direction(UIO1, pin, target);
	if (ret < 0) {
		shell_error(sh, "Change direction failed. UIO%d, Port:%d Direction:%d (%d)", UIO1, pin, target, ret);
		goto end;
	}
	shell_info(sh, "Change direction UIO%d Port:%d direction: %d", UIO1, pin, target);

	ret = gpio_direction(UIO2, pin, 0);
	if (ret < 0) {
		shell_error(sh, "Change direction failed. UIO%d, Port:%d Direction:%d (%d)", UIO2, pin, target, ret);
		goto end;
	}
	shell_info(sh, "Change direction UIO%d Port:%d direction: %d", UIO2, pin, target);

end:
	return ret;
}

static int gpio_verify_status(const struct shell *sh, uint8_t src_uio, uint8_t dst_uio, uint8_t pin, uint8_t target)
{
	int ret;
	uint8_t status;

	/* Change src status to Low */
	ret = gpio_write(src_uio, pin, target);
	if (ret < 0) {
		shell_error(sh, "Write status failed. UIO%d, Port:%d Status:%d (%d)", src_uio, pin, target, ret);
		goto end;
	}
	shell_info(sh, "Write UIO%d Port:%d Status: %d", src_uio, pin, target);

	/* Read dst Status */
	ret = gpio_read(dst_uio, pin, &status);
	if (ret < 0) {
		shell_error(sh, "Read status failed. UIO%d, Port:%d", dst_uio, pin);
		goto end;
	}
	shell_info(sh, "Read UIO%d Port:%d status : %d", dst_uio, pin, status);

	if (status != target) {
		shell_error(sh, "Read status unmatch. UIO%d, Port:%d, status:%d (expected: %d)", dst_uio, pin, status, target);
		ret = -EINVAL;
		goto end;
	}

end:
	return ret;
}

static int gpio_loop_back_test(const struct shell *sh, uint8_t src_uio, uint8_t dst_uio, uint8_t pin)
{
	int ret;
	uint8_t target;

	/* Change status Low and verify */
	ret = gpio_verify_status(sh, src_uio, dst_uio, pin, 0);
	if (ret < 0) {
		goto end;
	}

	/* Change status Hi and verify */
	ret = gpio_verify_status(sh, src_uio, dst_uio, pin, 1);
	if (ret < 0) {
		goto end;
	}

	/* Change status Low and verify */
	ret = gpio_verify_status(sh, src_uio, dst_uio, pin, 0);
	if (ret < 0) {
		goto end;
	}

end:
	return ret;
}

static int cmd_gpio_test(const struct shell *sh, size_t argc, char **argv)
{
	int ret = 0;
	uint8_t pin_num;
	char *endp;

	if (argc < 2) {
		shell_error(sh, "Usage: gpio test <pin_num>");
		ret = -EINVAL;
		goto end;
	}

	endp = NULL;
	pin_num = strtol(argv[1], &endp, 10);
	if ((endp == argv[1]) || (*endp != '\0')) {
		shell_error(sh, "Invalid uio: %s", argv[1]);
		ret = -EINVAL;
		goto end;
	}

	for (int pin = 0; pin < pin_num; pin++) {

		/* Change direction to input */
		ret = gpio_direction(UIO2, pin, 0);
		if (ret < 0) {
			break;
		}

		/* Change direction to output */
		ret = gpio_direction(UIO1, pin, 1);
		if (ret < 0) {
			break;
		}
	}

	for (int pin = 0; pin < pin_num; pin++) {
		shell_info(sh, "========= Start UIO1 -> UIO2 port: %d =========", pin);

		/* Loop back test */
		ret = gpio_loop_back_test(sh, UIO1, UIO2, pin);
		if (ret < 0) {
			break;
		}
	}

	for (int pin = 0; pin < pin_num; pin++) {

		/* Change direction to input */
		ret = gpio_direction(UIO1, pin, 0);
		if (ret < 0) {
			break;
		}

		/* Change direction to output */
		ret = gpio_direction(UIO2, pin, 1);
		if (ret < 0) {
			break;
		}
	}

	for (int pin = 0; pin < pin_num; pin++) {
		shell_info(sh, "========= Start UIO2 -> UIO1 port: %d =========", pin);

		/* Loop back test */
		ret = gpio_loop_back_test(sh, UIO2, UIO1, pin);
		if (ret < 0) {
			break;
		}
	}

end:
	return ret;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_gpio,
			       SHELL_CMD_ARG(read, NULL, "Read port status: gpio read <uio> <id>", cmd_gpio_read, 3, 0),
			       SHELL_CMD_ARG(write, NULL, "Write port status: gpio write <uio> <id> <status>", cmd_gpio_write, 4, 0),
			       SHELL_CMD_ARG(direction, NULL, "Change port direction: gpio direction <uio> <id> <direction>", cmd_gpio_direction, 4, 0),
			       SHELL_CMD_ARG(test, NULL, "Loopback test: gpio test <pin_num>", cmd_gpio_test, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(gpio, &sub_gpio, "GPIO test utilities", NULL);
