#include "rh_game_chest.h"

#include <stdlib.h>

#include "rh_game_core.h"
#include "rh_game_player.h"

#include "rh_game_bullet.h"

#include "buzzer.h"

rh_game_chest_t rh_chests[ROAD_MAX_CHESTS];
uint8_t rh_active_power = RH_POWER_NONE;
uint8_t rh_power_timer	= 0;
bool rh_shield_active = false;

void rh_game_chest_reset(void) {
	rh_active_power = RH_POWER_NONE;
	rh_power_timer	= 0;
	rh_shield_active = false;
	for (int i = 0; i < ROAD_MAX_CHESTS; i++) {
		rh_chests[i].active = false;
	}
}

void rh_game_chest_spawn(void) {
	for (int i = 0; i < ROAD_MAX_CHESTS; i++) {
		if (!rh_chests[i].active) {
			rh_chests[i].active = true;
			rh_chests[i].x		 = 128;
			rh_chests[i].lane	 = rand() % ROAD_LANE_COUNT;
			break;
		}
	}
}

bool rh_game_consume_shield(void) {
	if (rh_shield_active) {
		rh_shield_active = false;
		if (rh_active_power == RH_POWER_SHIELD || rh_active_power == RH_POWER_NONE) {
			rh_active_power = RH_POWER_NONE;
			rh_power_timer	= 0;
		}
		return true;
	}
	return false;
}

static uint8_t rh_game_get_random_power(bool allow_shield) {
	static const uint8_t powers_with_shield[] = {
		RH_POWER_GATLING,
		RH_POWER_PIERCING,
		RH_POWER_EXPLOSIVE,
		RH_POWER_SPIDER,
		RH_POWER_SHIELD
	};
	static const uint8_t powers_without_shield[] = {
		RH_POWER_GATLING,
		RH_POWER_PIERCING,
		RH_POWER_EXPLOSIVE,
		RH_POWER_SPIDER
	};
	const uint8_t *powers = allow_shield ? powers_with_shield : powers_without_shield;
	int count = allow_shield ? (sizeof(powers_with_shield) / sizeof(powers_with_shield[0])) : (sizeof(powers_without_shield) / sizeof(powers_without_shield[0]));
	return powers[rand() % count];
}

const char *rh_game_chest_get_power_name(uint8_t power) {
	switch (power) {
	case RH_POWER_GATLING:
		return "Rapid";
	case RH_POWER_PIERCING:
		return "Pierce";
	case RH_POWER_EXPLOSIVE:
		return "Boom";
	case RH_POWER_SPIDER:
		return "Freeze";
	case RH_POWER_SHIELD:
		return "Shield";
	default:
		return "";
	}
}

bool rh_game_chest_update(int move_speed) {
	if (rh_power_timer > 0) {
		rh_power_timer--;
		if (rh_power_timer == 0) {
			rh_shield_active = false;
			rh_active_power = RH_POWER_NONE;
			/* reset shoot timer so normal fire resumes immediately */
			rh_game_bullet_reset_shoot_timer();
		}
	}

	for (int i = 0; i < ROAD_MAX_CHESTS; i++) {
		if (!rh_chests[i].active)
			continue;
		rh_chests[i].x -= move_speed;
		if (rh_chests[i].x < -10) {
			rh_chests[i].active = false;
			continue;
		}
		if (rh_chests[i].lane == rh_player_lane &&
			rh_chests[i].x <= ROAD_PLAYER_X + ROAD_PLAYER_W &&
			rh_chests[i].x + 8 >= ROAD_PLAYER_X) {
			rh_chests[i].active = false;
			if (rh_empower_active) {
				/* while "Cuong hoa" is active the player just smashes through
				   chests without picking up a new power */
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
				return true;
			}
			if (rh_shield_active) {
				/* while shield is active, picking a chest refreshes shield and grants another power */
				rh_shield_active = true;
				rh_power_timer = 100;
				rh_active_power = rh_game_get_random_power(false);
				rh_game_bullet_reset_shoot_timer();
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
				}
				return true;
			}
			if (rh_power_timer > 0) {
				/* Do not extend active power time when picking a chest */
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
				return true;
			}
			/* activate a new power when none is currently active */
			rh_shield_active = false;
			rh_active_power = rh_game_get_random_power(true);
			rh_power_timer	= (rh_active_power == RH_POWER_SHIELD) ? 100 : 70;
			rh_shield_active = (rh_active_power == RH_POWER_SHIELD);
			/* ensure autoshoot fires immediately in new mode */
			rh_game_bullet_reset_shoot_timer();
			/* no drone power from chests; drones only spawn from Mode */
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
			}
			return true;
		}
	}
	return false;
}
