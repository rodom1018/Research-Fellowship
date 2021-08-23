#include "contiki.h"
#include "net/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

static int count=0;
static int first[3];
static int second[3];

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
  printf("out of if : second[2] value :%d \n", second[2]);
  if(count==0){
	first[0]=from->u8[0];
	first[1]=from->u8[1];
	first[2]=0;
	count++;
  }else if(count==1){
	if(first[0] != from->u8[0] && first[1] != from->u8[1]){
		second[0]=from->u8[0];
		second[1]=from->u8[1];
		second[2]=0;
		count++;
	}else{
		first[2]=0;
	}
  }else if(count==2){
	if(first[0] != from->u8[0] && first[1] != from->u8[1]){
		second[2]=0;
	}else{
		first[2]=0;
	}
  }

  
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  leds_init();
  
  while(1) {

    count = 0 ;

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    //printf("broadcast message sent\n");

    etimer_set(&et, CLOCK_SECOND*2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(count==0){
        leds_off(LEDS_ALL);
        etimer_set(&et, CLOCK_SECOND*0.1);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	leds_on(LEDS_BLUE);
	leds_off(LEDS_GREEN);
	leds_off(LEDS_RED);
    }else if(count==1){
        leds_off(LEDS_ALL);
        etimer_set(&et, CLOCK_SECOND*0.1);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	leds_on(LEDS_BLUE);
	leds_on(LEDS_GREEN);
	leds_off(LEDS_RED);
    }else if(count==2){
        leds_off(LEDS_ALL);
        etimer_set(&et, CLOCK_SECOND*0.1);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	leds_on(LEDS_BLUE);
	leds_on(LEDS_GREEN);
	leds_on(LEDS_RED);
    }

    first[2]++;
    second[2]++;

    if(first[2]>=3 && count ==2 ){
	printf("first[2] value :%d", first[2]);
	first[0]=second[0];
	first[1]=second[1];
	first[2]=second[2];
        
	second[0] =0;
	second[1] =0;
	second[2] =0;
	
	count--;
    }

    if(first[2]>=3 && count ==1){
	printf("first[2] value :%d", first[2]);

	first[0] =0;
	first[1] =0;
	first[2] =0;
	count--;
    }

    if(second[2]>=3){
	printf("second[2] value :%d", second[2]);
	second[2]=0;
	count--;
    }


  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/