#ifndef __RH_GAME_CORE_H__
#define __RH_GAME_CORE_H__

#include "rh_game_common.h"

extern uint16_t rh_score;
extern uint8_t rh_game_state;
extern uint16_t rh_state_timer;
extern uint16_t rh_game_tick;
extern uint16_t rh_high_score;
extern bool rh_sound_enabled;
extern uint16_t rh_enemy_kill_counter;
extern uint8_t rh_empower_charge;
extern uint8_t rh_empower_timer;
extern bool rh_empower_active;

void rh_game_reset_round(void);
void rh_game_trigger_over(void);
void rh_game_update(void);
void rh_toggle_sound(void);
bool rh_is_sound_enabled(void);
const char *rh_game_get_power_name(uint8_t power);
bool rh_game_can_move_to_lane(uint8_t lane);
void rh_game_clear_stage_entities(void);
void rh_game_on_target_killed(bool is_tank);
void rh_game_add_empower_charge(uint8_t amount);

#endif //__RH_GAME_CORE_H__
