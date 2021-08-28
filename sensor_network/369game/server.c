#include "contiki.h"
#include "net/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

#include "final_project_both.h"
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t temp_hop=0;
uint8_t temp_addr1[10];
uint8_t temp_addr2[10];


typedef struct{
	uint8_t ttl;
	uint8_t addr1;
	uint8_t addr2;
	uint8_t route1[10];
	uint8_t route2[10];
}table;

table my_table[10];

uint8_t current_num=1;
uint8_t valid = 1;

route * temp_route;
send_game * temp_send_game;
recv_game * temp_recv_game;

int current_node=0;
int count=0;
int broad=0;
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  temp_route=temp_recv_game=temp_send_game=packetbuf_dataptr();
  
  //server does not need this packet:trash it!
  if(temp_route->type == 0) return;
  if(temp_send_game->type == 30) return;

  if(temp_route->type == 10){
  	int i = 0, j =0;
  	//check if there is already existing info. 
  	for(i=0 ; i<10 ; i++){	
		//match info.
		if(my_table[i].addr1 ==temp_route->addr1[temp_route->hop]){
			return;
		}
  	}

  	//check if there is new space.
  	for(i=0; i<10 ; i++){
		//empty space! store info.
		if(my_table[i].addr1 ==0){
	
			my_table[i].addr1 = temp_route->addr1[temp_route->hop];
			my_table[i].addr2 = temp_route->addr2[temp_route->hop];
			my_table[i].ttl = temp_route->hop;
			for(j=0 ; j<=temp_route->hop ; j++){
				my_table[i].route1[j] = temp_route->addr1[j];
				my_table[i].route2[j] = temp_route->addr2[j];
			}	
			return;
		}
  	}
  }

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

  if(temp_recv_game->type == 40){
	//valid packet?
	if(temp_recv_game->current_num != current_num) return;
	//if valid packet...valid flag =1
	valid = 1;
	current_num++;
	current_node++;
	printf("%d.%d says : %s\n", temp_recv_game->addr1, temp_recv_game->addr2, temp_recv_game->sentence);
  }
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  rimeaddr_node_addr.u8[0] = 1;
  rimeaddr_node_addr.u8[1] = 0;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {
    //make new route message, and broadcast it.
    if(broad>=1){
	broad=0;
	route send_route;
	send_route.hop = 0;
	send_route.type = 0;
	send_route.addr1[0] = rimeaddr_node_addr.u8[0];
	send_route.addr2[0] = rimeaddr_node_addr.u8[1];
        packetbuf_copyfrom(&send_route, sizeof(send_route));
        broadcast_send(&broadcast);
        printf("message sent(update routing table - 5 seconds....)\n");

        etimer_set(&et, CLOCK_SECOND * 5);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
    
    while(my_table[current_node].addr1 == 0) current_node++;

    if(current_node >= 10) current_node =0;
    //non empty space
    send_game send_msg;
    send_msg.current_num=current_num;
    send_msg.ttl = (my_table[current_node].ttl-1);
    send_msg.type = 30;
    send_msg.pointer = 1; 
    send_msg.addr1 = my_table[current_node].route1[my_table[current_node].ttl];
    send_msg.addr2 = my_table[current_node].route2[my_table[current_node].ttl];

	int j=0;
	for(j=0 ; j<=my_table[current_node].ttl ; j++){
		send_msg.route1[j] = my_table[current_node].route1[j];
		send_msg.route2[j] = my_table[current_node].route2[j];
	}
	valid=0;
	packetbuf_copyfrom(&send_msg, sizeof(send_msg));
        broadcast_send(&broadcast);

        etimer_set(&et, CLOCK_SECOND * 2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	if(valid==0){
		broad++;
		//this node lose 369 game. -> remove node info, init current_num(restart game)
		printf("=========================\n");
		printf("bomb! restart game!!\n\n\n");
		printf("=========================\n");

		my_table[current_node].addr1=0;
		my_table[current_node].addr2=0;
		current_num=1;
		current_node=0;
	}
    
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/