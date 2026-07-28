#include "rh_game_core.h"

#include <stdlib.h>

#include "rh_game_boss.h"
#include "rh_game_bullet.h"
#include "rh_game_chest.h"
#include "rh_game_enemy.h"
#include "rh_game_obstacle.h"
#include "rh_game_player.h"

#include "buzzer.h"

uint16_t rh_score	   = 0;
uint8_t rh_game_state  = RH_COMBAT_STATE_READY;
uint16_t rh_state_timer = 0;
uint16_t rh_game_tick  = 0;
uint16_t rh_high_score = 0;
uint16_t rh_enemy_kill_counter = 0;
bool rh_sound_enabled = true;
uint8_t rh_empower_charge = 0;
uint8_t rh_empower_timer = 0;
bool rh_empower_active = false;
static uint16_t rh_spawn_timer = 12;

void rh_game_trigger_over(void) {
	if (rh_score > rh_high_score) {
		rh_high_score = rh_score;
	}
	rh_game_state  = RH_COMBAT_STATE_GAME_OVER;
	rh_state_timer = RH_GAME_OVER_ANIM_TOTAL_TICKS;
	if (rh_sound_enabled) {
		BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
	}
}

void rh_toggle_sound(void) {
	rh_sound_enabled = !rh_sound_enabled;
}

bool rh_is_sound_enabled(void) {
	return rh_sound_enabled;
}

void rh_game_reset_round(void) {
	rh_score = 0;
	rh_game_state  = RH_COMBAT_STATE_PLAYING; /* Start goes straight into the game, no READY intro screen */
	rh_state_timer = 0;
	rh_game_tick   = 0;
	rh_spawn_timer = 12;
	rh_enemy_kill_counter = 0;
	rh_empower_charge = 0;
	rh_empower_timer = 0;
	rh_empower_active = false;

	rh_game_player_reset();
	rh_game_enemy_reset();
	rh_game_obstacle_reset();
	rh_game_bullet_reset();
	rh_game_chest_reset();
	rh_game_boss_reset();
}

const char *rh_game_get_power_name(uint8_t power) {
	return rh_game_chest_get_power_name(power);
}

bool rh_game_can_move_to_lane(uint8_t lane) {
	return true;
}

void rh_game_clear_stage_entities(void) {
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		rh_enemies[i].active = false;
	}
	for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
		rh_obstacles[i].active = false;
	}
	for (int i = 0; i < ROAD_MAX_CHESTS; i++) {
		rh_chests[i].active = false;
	}
	for (int i = 0; i < ROAD_MAX_ENEMY_BULLETS; i++) {
		rh_enemy_bullets[i].active = false;
	}
	for (int i = 0; i < ROAD_MAX_PLAYER_BULLETS; i++) {
		rh_player_bullets[i].active = false;
	}
	for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
		rh_boss_projectiles[i].active = false;
	}
}

void rh_game_add_empower_charge(uint8_t amount) {
	/* already empowered: ignore further charge, it will reset to 0 once the
	   current empower window ends and the player has to earn it again */
	if (rh_empower_active) {
		return;
	}
	if (rh_empower_charge < RH_EMPOWER_TANKS_TO_FULL) {
		rh_empower_charge += amount;
		if (rh_empower_charge >= RH_EMPOWER_TANKS_TO_FULL) {
			rh_empower_charge  = RH_EMPOWER_TANKS_TO_FULL;
			rh_empower_active  = true;
			rh_empower_timer   = RH_EMPOWER_DURATION;
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
			}
		}
	}
}

void rh_game_on_target_killed(bool is_tank) {
	if (is_tank) {
		rh_game_add_empower_charge(1);
	}
}

void rh_game_update(void) {
	int difficulty = 1 + (rh_game_tick / 140);
	int move_speed = ROAD_ENEMY_SPEED_BASE + difficulty;
	if (move_speed > 8) {
		move_speed = 8;
	}

	/* "Cuong hoa" (Empower): the world keeps scrolling while active (even
	   faster) so the player visibly rams through everything at speed,
	   rather than the screen pausing. */
	if (rh_empower_active) {
		rh_empower_timer--;
		if (rh_empower_timer == 0) {
			rh_empower_active = false;
			rh_empower_charge = 0;
		}
		else {
			move_speed += RH_EMPOWER_SPEED_BOOST;
		}
	}

	if (rh_game_boss_active()) {
		/* enforce default bullets for the entire boss encounter
		   (covers INTRO, FIGHT and OUTRO for all 3 boss types:
		   land / sea / sky).  Reset every tick so any residual
		   power state from before the fight can never bleed in. */
		rh_active_power  = RH_POWER_NONE;
		rh_power_timer   = 0;
		rh_shield_active = false;

		if (rh_game_boss_update(move_speed))
			return;

		rh_game_bullet_autoshoot();
		if (rh_game_bullet_update())
			return;

		rh_game_boss_check_bullet_hits();

		rh_game_explosion_update();
		rh_game_tick++;
		return;
	}

	if (rh_boss_countdown > 0) {
		rh_boss_countdown--;
		if (rh_boss_countdown == 0) {
			rh_game_boss_start_next();
			rh_game_tick++;
			return;
		}
	}

	if (rh_spawn_timer > 0) {
		rh_spawn_timer--;
	}
	else {
		rh_spawn_timer = 9 - (difficulty / 2);
		if (rh_spawn_timer < 4) {
			rh_spawn_timer = 4;
		}
		int roll = rand() % 100;
		if (roll < 23) {
			rh_game_enemy_spawn();
		}
		else if (roll < 30) {
			rh_game_enemy_spawn_tank();
		}
		else if (roll < 55) {
			rh_game_obstacle_spawn();
		}
		else if (roll < 65) {
			rh_game_chest_spawn();
		}
	}

	if (rh_game_enemy_update(move_speed))
		return;
	if (rh_game_obstacle_update(move_speed))
		return;
	if (rh_game_chest_update(move_speed))
		return;

	if (rh_active_power == RH_POWER_LASER) {
		rh_game_laser_update();
	}

	rh_game_bullet_autoshoot();
	if (rh_game_bullet_update())
		return;

	rh_game_explosion_update();
	rh_game_tick++;
}