#ifndef __RH_GAME_PLAYER_H__
#define __RH_GAME_PLAYER_H__

#include "rh_game_common.h"

extern uint8_t rh_player_lane;

void rh_game_player_reset(void);
void rh_game_player_move_up(void);
void rh_game_player_move_down(void);

#endif //__RH_GAME_PLAYER_H__
