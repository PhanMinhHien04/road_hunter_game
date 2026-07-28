#ifndef __RH_GAME_CHEST_H__
#define __RH_GAME_CHEST_H__

#include "rh_game_common.h"

extern rh_game_chest_t rh_chests[ROAD_MAX_CHESTS];
extern uint8_t rh_active_power;
extern uint8_t rh_power_timer;
extern bool rh_shield_active;

void rh_game_chest_reset(void);
void rh_game_chest_spawn(void);
bool rh_game_chest_update(int move_speed);
bool rh_game_consume_shield(void);
const char *rh_game_chest_get_power_name(uint8_t power);

#endif //__RH_GAME_CHEST_H__
