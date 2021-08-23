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
#include "sys/node-id.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <clock.h>

#include "dev/serial-line.h"
#include "sn3_both.h"
my_struct * my;

int hour;
int min;
int sec;

int mote2[2]={0,0};
int mote3[2]={0,0};
int mote4[2]={0,0};
/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  leds_on(LEDS_BLUE);
//
  my=packetbuf_dataptr();
  char sentence[100] = "";
  if(mote2[0] ==0 && mote2[1]==0){

	mote2[0] = from->u8[0];
	mote2[1] = from->u8[1];
	strcat(sentence, "<mote2>");

  }else if(mote2[0] == from->u8[0] && mote2[1] == from->u8[1]){
	
	strcat(sentence, "<mote2>");

  }else if(mote3[0] ==0 && mote3[1]==0){

	mote3[0] = from->u8[0];
	mote3[1] = from->u8[1];
	strcat(sentence, "<mote3>");

  }else if(mote3[0] == from->u8[0] && mote3[1] == from->u8[1]){

	strcat(sentence, "<mote3>");

  }else if(mote4[0] ==0 && mote4[1]==0){

	mote4[0] = from->u8[0];
	mote4[1] = from->u8[1];
	strcat(sentence, "<mote4>");
  
  }else if(mote4[0] == from->u8[0] && mote4[1] == from->u8[1]){

	strcat(sentence, "<mote4>");
  }

  
  /*printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);*/
//
  clock_time_t time = (unsigned int)clock_time()/128;
  //printf("time: %u\n", time);

  int temp_hour=hour;
  int temp_min=min;
  int temp_sec=sec;
  temp_sec=temp_sec + (int)time;
  
  while(temp_sec>=60){
	temp_sec-=60;
	temp_min+=1;
  }

  while(temp_min>=60){
	temp_min-=60;
	temp_hour+=1;
  }
  while(temp_hour>=24){
	temp_hour-=24;
  }

  printf(sentence);
  printf("[%d:%d:%d]", temp_hour, temp_min, temp_sec);

//
  printf("[%u] temp: %u.%u C, battery: %u.%u V",my->seq, my->temp1, my->temp2, my->bat1, my->bat2);
  printf("\n");
  //printf("temp: %u %u %u %u %u\n", my->seq, my->temp1, my->temp2, my->bat1, my->bat2);

}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  unsigned short id = 1;
//
//
  rimeaddr_node_addr.u8[0]=1;
  rimeaddr_node_addr.u8[1]=0;

  PROCESS_BEGIN();

//hour
  PROCESS_WAIT_EVENT();
  if (ev == serial_line_event_message && data != NULL) {
     //printf("got input string: '%s'\n", (const char *) data);
     hour=atoi(data);
  }

//minute
  PROCESS_WAIT_EVENT();
  if (ev == serial_line_event_message && data != NULL) {
     min=atoi(data);
  }

//second
  PROCESS_WAIT_EVENT();
  if (ev == serial_line_event_message && data != NULL) {
     sec=atoi(data);
  }

  printf("data : %d\n", hour);
  printf("data : %d\n", min);
  printf("data : %d\n", sec);

  unicast_open(&uc, 26, &unicast_callbacks);

  node_id_burn(id);
  
  while(1) {

    static struct etimer et;
    rimeaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 5);
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/