// ----------------------------------------------------------------------------
// Copyright 2016-2019 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#include "AppPower.h"
#include "platform/mbed_debug.h"
#include "DigitalOut.h"
#include "platform/mbed_wait_api.h"

void avnet_bg96_power(void)
{
    mbed::DigitalOut BG96_RESET(MBED_CONF_APP_BG96_RESET, 1);
    mbed::DigitalOut BG96_PWRKEY(MBED_CONF_APP_BG96_PWRKEY, 0);
    //	mbed::DigitalOut BG96_VBAT_3V8_EN(D11, 0);
    //  3V8_EN is pulled to 5V and not connected as the reg is always enabled
    //	BG96_VBAT_3V8_EN = 1;
    BG96_PWRKEY = 1;

    wait_ms(300);

    BG96_RESET = 0;
}

void mikroe_bg96_power(void)
{
/* MikroE BG96 Module details 
   The Reset line on BG96 is not connected.
   The PWRKEY pin is routed to the mikroBUS™ RST pin. A LOW pulse on this pin for at least 100ms will toggle the power status of the device. 
   If powered down, and the valid power supply voltage is present, a pulse on this pin will power up the device. The successful action will 
   be indicated by the STAT LED and the STAT (AN) pin. If the device is already powered up, a pulse on this pin will power the module down. 
   It is also possible to power down the module by issuing the AT+QPOWD command. Both methods are considered as safe and will let the module 
   log off from the network and allow the firmware to save important data, before completely disconnecting the power supply. An abrupt loss 
   of power might lead to unwanted consequences.
*/
	mbed::DigitalOut BG96_PWRKEY(MBED_CONF_APP_BG96_PWRKEY, 1);

	BG96_PWRKEY = 0;

	wait_ms(300);

	BG96_PWRKEY = 1;
}

void init_cellular_power(void)
{
    printf("%s\r\n", __FUNCTION__);
	if(MBED_CONF_APP_PWR_MIKROE)
	{
		printf("Powering MikroE module\n");
		mikroe_bg96_power();
	}else{
		printf("Powering non-MikroE module\n");
	    avnet_bg96_power();
	}
    wait_ms(5000); // wait for the modem to establish a connection
    printf("done\n");
}

extern "C" void mbed_main(void)
{
    printf("%s\r\n", __FUNCTION__);
    init_cellular_power();
}
