typedef struct{
	uint8_t type;
	uint8_t pointer;
	uint8_t hop;
	uint8_t addr1[10];
	uint8_t addr2[10];
}route;

typedef struct{
	uint8_t type;
	uint8_t pointer;

	uint8_t route1[10];
	uint8_t route2[10];
	uint8_t ttl;
	uint8_t current_num;

	uint8_t addr1;
	uint8_t addr2;
}send_game;

typedef struct{
	uint8_t type; 
	uint8_t pointer;

	uint8_t current_num;
	char sentence[15];
	uint8_t ttl;
	uint8_t route1[10];
	uint8_t route2[10];

	uint8_t addr1;
	uint8_t addr2;
}recv_game;