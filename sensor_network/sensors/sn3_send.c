/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"

#include "dev/sht11.h"
#include "dev/battery-sensor.h"
#include "lib/sensors.h"

#include "dev/leds.h"

#include "sn3_both.h"
#include <stdio.h>



my_struct info;
/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  unicast_open(&uc, 26, &unicast_callbacks);
  
  sht11_init();
  SENSORS_ACTIVATE(battery_sensor);
  static unsigned int seq=0;

  while(1) {
    static struct etimer et;
    rimeaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND*2);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

//temperature.
    unsigned int temp = sht11_temp(); 
    double temp1 = -39.6+0.01*temp;
    int temp2 = (unsigned int) temp1;
    double temp3= temp1-temp2;
    unsigned int temp4=(unsigned int)(temp3*100);
    info.temp1 = temp2;
    info.temp2 = temp4;
    //printf("temp: %d.%.2u C\n", temp2, (unsigned int)(temp3*100));

//battery.
    unsigned int bat = battery_sensor.value(0);
    double bat1 = 1.223*bat/1024;
    unsigned int bat2 = (unsigned int) bat1;
    double bat3 = bat1-bat2;
    unsigned int bat4 = (unsigned int)(bat3*100);
    info.bat1 = bat2;
    info.bat2 = bat4;
    //printf("ADC value : %d.%.2u\n", bat2, (unsigned int)(bat3*100));
    seq = seq+1;
    printf("seq: %d \n", seq);
    info.seq = seq;


//send message.
    packetbuf_copyfrom(&info, sizeof(info));
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    printf("info:%u %u %u %u %u\n", info.seq, info.temp1, info.temp2, info.bat1, info.bat2);
    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/