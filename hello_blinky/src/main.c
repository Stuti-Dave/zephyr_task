/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

int main(void)
{
	uint8_t led_state=is_blinking();
	if(led_state==0){
		printf("Hello World! from  %s\n", CONFIG_BOARD_TARGET);
	}

	return 0;
}
