
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
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/


uint8_t temp_hop=100;
uint8_t temp_addr1[10];
uint8_t temp_addr2[10];


route * temp_route;
send_game * temp_send_game;
recv_game * temp_recv_game;

route send_route;
recv_game recv_game_clone;
send_game send_game_clone;
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  printf("broadcast message received from %d.%d\n",from->u8[0], from->u8[1]);
  temp_route=temp_recv_game=temp_send_game=packetbuf_dataptr();
  printf("message type : %d\n", temp_send_game->type);
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//when child node send info.
  if(temp_route->type == 10){
	if(temp_route->addr1[temp_route->pointer] == rimeaddr_node_addr.u8[0]){

		int i =0;
		for(i=0 ; i<=temp_route->hop ; i++){
			send_route.addr1[i] = temp_route->addr1[i];
			send_route.addr2[i] = temp_route->addr2[i];
        	}
		send_route.pointer = temp_route->pointer-1;
		send_route.type = 10;
		send_route.hop = temp_route->hop;

		packetbuf_copyfrom(&send_route, sizeof(send_route));
		broadcast_send(&broadcast);

		return;
	}else{
		//trash message.
		return;
	}
  }
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
  if(temp_route->type == 0){
  	if(temp_route->hop < temp_hop){
		//save new route.
		temp_route->hop ++;
		temp_hop = temp_route->hop;
		send_route.hop = temp_route->hop;
		int i =0;
		for(i=0 ; i<temp_route->hop ; i++){
			send_route.addr1[i] = temp_route->addr1[i];
			send_route.addr2[i] = temp_route->addr2[i];
        	}
		send_route.addr1[temp_route->hop] = rimeaddr_node_addr.u8[0];
		send_route.addr2[temp_route->hop] = rimeaddr_node_addr.u8[1];
		send_route.pointer = temp_route->hop-1;
		send_route.type = 0;
		//broadcast (to next hop client)
		packetbuf_copyfrom(&send_route, sizeof(send_route));
		broadcast_send(&broadcast);
		//unicast to server or parents. ( send my info.)
		send_route.type = 10;
		packetbuf_copyfrom(&send_route, sizeof(send_route));
		broadcast_send(&broadcast);
  	} 
  }


  //////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
  if(temp_send_game->type == 30){
	if(temp_send_game -> ttl == 0 ){
	//check destination address = my address
	//if not, drop it.		

		if(temp_send_game->addr2==rimeaddr_node_addr.u8[1]){
			recv_game_clone.ttl = temp_hop-1;
			recv_game_clone.pointer= temp_hop;
			recv_game_clone.current_num = temp_send_game->current_num;
			recv_game_clone.type=40;
			recv_game_clone.addr1 = temp_send_game->addr1; 
			recv_game_clone.addr1 = temp_send_game->addr2;
			
			int i=0;
			while(temp_send_game->route1[i] !=temp_send_game->addr1){
				recv_game_clone.route1[i] = temp_send_game->route1[i];
				recv_game_clone.route2[i] = temp_send_game->route2[i];
				i++;
			}
			i++;
			recv_game_clone.route1[i] = temp_send_game->route1[i];
			recv_game_clone.route2[i] = temp_send_game->route2[i];
			//////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////
			int temp_num = temp_send_game->current_num;
			int clap =0;
			if(temp_num/10 ==3 || temp_num/10 == 6 || temp_num /10 == 9){
				clap++;
			}

			if(temp_num%10 ==3 || temp_num%10 == 6 || temp_num%10 ==9){
				clap++;
			}
			///////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////

			if(clap ==0){
				sprintf(recv_game_clone.sentence, "%d", temp_num);
			}else if(clap ==1){
				strcpy(recv_game_clone.sentence,"clap!");
			}else{
				strcpy(recv_game_clone.sentence,"clap!clap!"); 
			}

			packetbuf_copyfrom(&recv_game_clone, sizeof(recv_game_clone));
			broadcast_send(&broadcast);

			return;
			
		}else return; 
	}else{

		//ttl is not 0
		//not valid address:trash
		if(temp_send_game->route1[temp_send_game->pointer] != rimeaddr_node_addr.u8[0]) return;

		//middle transmitter is me!
		send_game_clone.type=30;
		send_game_clone.pointer = temp_send_game->pointer+1;
		send_game_clone.ttl = temp_send_game->ttl-1;
		send_game_clone.addr1 = temp_send_game->addr1;
		send_game_clone.addr2 = temp_send_game->addr2;
		send_game_clone.current_num = temp_send_game->current_num;

		int i =0;
		while(temp_send_game->route1[i]!=temp_send_game->addr1){
			send_game_clone.route1[i] = temp_send_game->route1[i];
			send_game_clone.route2[i] = temp_send_game->route2[i];
			i++;
		}
		i++;
		send_game_clone.route1[i] = temp_send_game->addr1;
		send_game_clone.route2[i] = temp_send_game->addr2;	

 		packetbuf_copyfrom(&send_game_clone, sizeof(send_game_clone));
		broadcast_send(&broadcast);

		return;
	}	
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
  if(temp_recv_game->type == 40){
	//packet live finished

	if(temp_recv_game->ttl ==0){
		 return;
	}
	//correct destination?
	if(temp_recv_game->route1[temp_recv_game->ttl] != rimeaddr_node_addr.u8[0]){
		 return;
	}

	recv_game_clone.ttl = temp_recv_game->ttl-1;
	recv_game_clone.pointer = temp_recv_game->pointer;
	recv_game_clone.current_num = temp_recv_game->current_num;
	recv_game_clone.addr1=temp_recv_game->addr1;
	recv_game_clone.addr2=temp_recv_game->addr2;

	strcpy(recv_game_clone.sentence, temp_recv_game->sentence);

	recv_game_clone.type=40;	
			
	int i=0;
	while(temp_send_game->route1[i] ==temp_send_game->addr1){
		recv_game_clone.route1[i] = temp_send_game->route1[i];
		recv_game_clone.route2[i] = temp_send_game->route2[i];
		i++;
	}

	i++;
	send_game_clone.route1[i] = temp_send_game->addr1;
	send_game_clone.route2[i] = temp_send_game->addr2;
	
 	packetbuf_copyfrom(&recv_game_clone, sizeof(recv_game_clone));
	broadcast_send(&broadcast);

	return;
  }    
}



/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD( example_broadcast_process , ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    //timer.
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/