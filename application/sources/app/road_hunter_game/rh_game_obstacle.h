#ifndef __RH_GAME_OBSTACLE_H__
#define __RH_GAME_OBSTACLE_H__

#include "rh_game_common.h"

extern rh_game_obstacle_t rh_obstacles[ROAD_MAX_OBSTACLES];

void rh_game_obstacle_reset(void);
void rh_game_obstacle_spawn(void);
bool rh_game_obstacle_update(int move_speed);

#endif //__RH_GAME_OBSTACLE_H__
