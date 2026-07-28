#ifndef __RH_GAME_ENEMY_H__
#define __RH_GAME_ENEMY_H__

#include "rh_game_common.h"

extern rh_game_enemy_t rh_enemies[ROAD_MAX_ENEMIES];

void rh_game_enemy_reset(void);
void rh_game_enemy_spawn(void);
void rh_game_enemy_spawn_tank(void);
bool rh_game_enemy_update(int move_speed);

#endif //__RH_GAME_ENEMY_H__
