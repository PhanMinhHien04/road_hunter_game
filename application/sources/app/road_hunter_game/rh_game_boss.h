#ifndef __RH_GAME_BOSS_H__
#define __RH_GAME_BOSS_H__

#include "rh_game_common.h"

extern rh_game_boss_t rh_boss;
extern rh_boss_projectile_t rh_boss_projectiles[ROAD_MAX_BOSS_PROJECTILES];
extern uint16_t rh_boss_countdown;

/* Reset all boss state for a new round (call from rh_game_reset_round). */
void rh_game_boss_reset(void);

/* True whenever a boss encounter (intro/fight/outro) is in progress. While
   true, the normal enemy/obstacle/chest spawner is skipped. */
bool rh_game_boss_active(void);

/* Begin the next boss in the LAND -> SEA -> SKY -> repeat cycle. Clears the
   road of every enemy/obstacle/chest/bullet first. */
void rh_game_boss_start_next(void);

/* Advance boss state (intro slide-in, attacks, outro) and move/collide its
   projectiles against the player. Returns true if it triggered game over. */
bool rh_game_boss_update(int move_speed);

/* Check active player bullets against the boss body/weak-point and against
   its shootable projectiles (land shells). Call after player bullets have
   moved for this tick. */
void rh_game_boss_check_bullet_hits(void);

#endif //__RH_GAME_BOSS_H__
