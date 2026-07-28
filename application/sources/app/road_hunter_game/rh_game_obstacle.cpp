#include "rh_game_obstacle.h"

#include <stdlib.h>

#include "rh_game_chest.h"
#include "rh_game_core.h"
#include "rh_game_player.h"

#include "rh_game_bullet.h"

#include "buzzer.h"

rh_game_obstacle_t rh_obstacles[ROAD_MAX_OBSTACLES];

void rh_game_obstacle_reset(void) {
	for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
		rh_obstacles[i].active = false;
	}
}

void rh_game_obstacle_spawn(void) {
	int type = rand() % 6;
	if (type == 3) {
		int lanes[5] = {0, 1, 2, 3, 4};
		for (int i = 4; i > 0; i--) {
			int j = rand() % (i + 1);
			int temp = lanes[i];
			lanes[i] = lanes[j];
			lanes[j] = temp;
		}
		for (int i = 0; i < 3; i++) {
			for (int o = 0; o < ROAD_MAX_OBSTACLES; o++) {
				if (!rh_obstacles[o].active) {
					rh_obstacles[o].active = true;
					rh_obstacles[o].x		= 128;
					rh_obstacles[o].lane	= lanes[i];
					rh_obstacles[o].type	= 3;
					rh_obstacles[o].hp		= 0;
					break;
				}
			}
		}
	}
	else {
		for (int o = 0; o < ROAD_MAX_OBSTACLES; o++) {
			if (!rh_obstacles[o].active) {
				rh_obstacles[o].active = true;
				rh_obstacles[o].x		= 128;
				rh_obstacles[o].lane	= rand() % ROAD_LANE_COUNT;
				rh_obstacles[o].type	= type;
				if (type == 4) rh_obstacles[o].hp = 1;
				else if (type == 5) rh_obstacles[o].hp = 2;
				else rh_obstacles[o].hp = 0;
				break;
			}
		}
	}
}

bool rh_game_obstacle_update(int move_speed) {
	for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
		if (!rh_obstacles[i].active)
			continue;

		int obs_speed = move_speed;
		if (rh_obstacles[i].type == 1) {
			obs_speed = move_speed + 2;
		}
		else if (rh_obstacles[i].type == 2) {
			obs_speed = move_speed - 1;
			if (obs_speed < 1)
				obs_speed = 1;
		}
		else if (rh_obstacles[i].type == 4) {
			obs_speed = move_speed + 3;
		}
		else if (rh_obstacles[i].type == 5) {
			obs_speed = move_speed - 1;
			if (obs_speed < 1)
				obs_speed = 1;
		}
		rh_obstacles[i].x -= obs_speed;

		int obs_w = RH_OBSTACLE_W_FENCE;
		if (rh_obstacles[i].type == 0)
			obs_w = 8;
		else if (rh_obstacles[i].type == 1)
			obs_w = 8;
		else if (rh_obstacles[i].type == 2)
			obs_w = 16;
		else if (rh_obstacles[i].type == 4)
			obs_w = 10;
		else if (rh_obstacles[i].type == 5)
			obs_w = RH_OBSTACLE_W_TRUCK;

		if (rh_obstacles[i].x < -(obs_w + 2)) {
			rh_obstacles[i].active = false;
			continue;
		}
		if (rh_obstacles[i].lane == rh_player_lane &&
			rh_obstacles[i].x <= ROAD_PLAYER_X + ROAD_PLAYER_W &&
			rh_obstacles[i].x + obs_w >= ROAD_PLAYER_X) {
			if (rh_empower_active) {
				rh_obstacles[i].active = false;
				/* award score for destroyed vehicles */
				if (rh_obstacles[i].type == 4 || rh_obstacles[i].type == 5) {
					rh_score += rh_obstacle_score_value(&rh_obstacles[i]);
					rh_game_explosion_spawn(rh_obstacles[i].x + 4, rh_obstacles[i].lane);
				}
			}
			else if (rh_game_consume_shield()) {
				rh_obstacles[i].active = false;
				if (rh_obstacles[i].type == 4 || rh_obstacles[i].type == 5) {
					rh_score += rh_obstacle_score_value(&rh_obstacles[i]);
					rh_game_explosion_spawn(rh_obstacles[i].x + 4, rh_obstacles[i].lane);
				}
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
