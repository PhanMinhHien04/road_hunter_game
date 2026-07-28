#include "rh_game_enemy.h"

#include <stdlib.h>

#include "rh_game_chest.h"
#include "rh_game_core.h"
#include "rh_game_player.h"

#include "rh_game_bullet.h"

#include "buzzer.h"

rh_game_enemy_t rh_enemies[ROAD_MAX_ENEMIES];

void rh_game_enemy_reset(void) {
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		rh_enemies[i].active = false;
		rh_enemies[i].x = -100;
		rh_enemies[i].lane = 0;
		rh_enemies[i].hp = 0;
		rh_enemies[i].fire_timer = 0;
		rh_enemies[i].rockets_fired = 0;
		rh_enemies[i].trapped = false;
		rh_enemies[i].trap_timer = 0;
		rh_enemies[i].is_tank = false;
	}
}

void rh_game_enemy_spawn(void) {
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		if (!rh_enemies[i].active) {
			rh_enemies[i].x = ROAD_SCREEN_WIDTH;
			rh_enemies[i].lane = rand() % ROAD_LANE_COUNT;
			rh_enemies[i].hp = 1;
			rh_enemies[i].fire_timer = 0;
			rh_enemies[i].rockets_fired = 0;
			rh_enemies[i].trapped = false;
			rh_enemies[i].trap_timer = 0;
			rh_enemies[i].is_tank = false;
			rh_enemies[i].active = true;
			break;
		}
	}
}

void rh_game_enemy_spawn_tank(void) {
	/* only allow one tank on screen at a time */
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		if (rh_enemies[i].active && rh_enemies[i].is_tank)
			return;
	}
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		if (!rh_enemies[i].active) {
			rh_enemies[i].x = ROAD_SCREEN_WIDTH;
			rh_enemies[i].lane = rand() % ROAD_LANE_COUNT;
			rh_enemies[i].hp = RH_TANK_HP;
			rh_enemies[i].fire_timer = RH_TANK_BURST_INTERVAL;
			rh_enemies[i].rockets_fired = 0;
			rh_enemies[i].trapped = false;
			rh_enemies[i].trap_timer = 0;
			rh_enemies[i].is_tank = true;
			rh_enemies[i].active = true;
			break;
		}
	}
}

static void rh_game_tank_try_fire(rh_game_enemy_t *tank) {
	if (tank->trapped)
		return;
	if (tank->fire_timer > 0) {
		tank->fire_timer--;
		return;
	}
	if (tank->rockets_fired < RH_TANK_BURST_COUNT) {
		for (int j = 0; j < ROAD_MAX_ENEMY_BULLETS; j++) {
			if (!rh_enemy_bullets[j].active) {
				rh_enemy_bullets[j].active	  = true;
				rh_enemy_bullets[j].x		  = tank->x - 2;
				rh_enemy_bullets[j].lane	  = tank->lane;
				rh_enemy_bullets[j].is_enemy = true;
				rh_enemy_bullets[j].type	  = RH_POWER_EXPLOSIVE;
				rh_enemy_bullets[j].damage	  = 1;
				rh_enemy_bullets[j].piercing = false;
				rh_enemy_bullets[j].vx		  = 0;
				rh_enemy_bullets[j].vy		  = 0;
				break;
			}
		}
		tank->rockets_fired++;
		tank->fire_timer = RH_TANK_BURST_INTERVAL;
	}
	else {
		tank->rockets_fired = 0;
		tank->fire_timer	 = RH_TANK_COOLDOWN;
	}
}

bool rh_game_enemy_update(int move_speed) {
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		if (!rh_enemies[i].active)
			continue;
		if (rh_enemies[i].trapped) {
			rh_enemies[i].trap_timer--;
			if (rh_enemies[i].trap_timer == 0) {
				rh_enemies[i].trapped = false;
			}
		}
		/* tanks roll a bit slower than regular enemies, giving them time to fire */
		int enemy_speed = move_speed;
		if (rh_enemies[i].is_tank) {
			enemy_speed = move_speed - 1;
			if (enemy_speed < 1)
				enemy_speed = 1;
		}
		rh_enemies[i].x -= (rh_enemies[i].trapped ? 1 : enemy_speed);
		int enemy_w = rh_enemy_width(&rh_enemies[i]);
		if (rh_enemies[i].x + enemy_w < 0) {
			rh_enemies[i].active = false;
			continue;
		}
		if (rh_enemies[i].is_tank) {
			rh_game_tank_try_fire(&rh_enemies[i]);
		}
		if (rh_enemies[i].lane == rh_player_lane &&
			rh_enemies[i].x <= ROAD_PLAYER_X + ROAD_PLAYER_W &&
			rh_enemies[i].x + enemy_w >= ROAD_PLAYER_X) {
			if (rh_empower_active) {
				rh_enemies[i].active = false;
				rh_score += rh_enemy_score_value(&rh_enemies[i]);
				rh_game_explosion_spawn(rh_enemies[i].x + (enemy_w/2), rh_enemies[i].lane);
			}
			else if (rh_game_consume_shield()) {
				rh_enemies[i].active = false;
				rh_score += rh_enemy_score_value(&rh_enemies[i]);
				rh_game_explosion_spawn(rh_enemies[i].x + (enemy_w/2), rh_enemies[i].lane);
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			else {
				rh_game_trigger_over();
				return true;
			}

			
		}
	}
	return false;
}
