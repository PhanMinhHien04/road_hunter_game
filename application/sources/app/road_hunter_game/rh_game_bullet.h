#ifndef __RH_GAME_BULLET_H__
#define __RH_GAME_BULLET_H__

#include "rh_game_common.h"

extern rh_game_bullet_t rh_player_bullets[ROAD_MAX_PLAYER_BULLETS];
extern rh_game_bullet_t rh_enemy_bullets[ROAD_MAX_ENEMY_BULLETS];

void rh_game_bullet_reset(void);
void rh_game_bullet_fire(void);
void rh_game_bullet_autoshoot(void);
void rh_game_bullet_handle_mode_input(void);
bool rh_game_bullet_update(void);
void rh_game_bullet_explode(int bullet_index);
void rh_game_bullet_reset_shoot_timer(void);
void rh_game_explosion_update(void);
void rh_game_explosion_display(void);
void rh_game_laser_update(void);
void rh_game_laser_display(void);
void rh_game_explosion_spawn(int16_t x, uint8_t lane);

#endif //__RH_GAME_BULLET_H__
