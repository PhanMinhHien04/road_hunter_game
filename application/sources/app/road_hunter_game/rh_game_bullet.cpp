#include "rh_game_bullet.h"

#include "rh_game_chest.h"
#include "rh_game_core.h"
#include "rh_game_enemy.h"
#include "rh_game_obstacle.h"
#include "rh_game_player.h"
#include "view_render.h"

#include "buzzer.h"
#include "io_cfg.h"
#include "button.h"

rh_game_bullet_t rh_player_bullets[ROAD_MAX_PLAYER_BULLETS];
rh_game_bullet_t rh_enemy_bullets[ROAD_MAX_ENEMY_BULLETS];

#define ROAD_MAX_EXPLOSIONS 4

typedef struct {
	int16_t x;
	uint8_t lane;
	bool active;
	uint8_t timer;
} rh_game_explosion_t;

static rh_game_explosion_t rh_explosions[ROAD_MAX_EXPLOSIONS];
static uint16_t rh_shoot_timer = 0;

void rh_game_bullet_reset_shoot_timer(void) {
	rh_shoot_timer = 0;
}

void rh_game_explosion_spawn(int16_t x, uint8_t lane);
void rh_game_explosion_update(void);
void rh_game_explosion_display(void);

void rh_game_bullet_reset(void) {
	rh_shoot_timer = 0;
	for (int i = 0; i < ROAD_MAX_PLAYER_BULLETS; i++) {
		rh_player_bullets[i].active = false;
		rh_player_bullets[i].type = RH_POWER_NONE;
		rh_player_bullets[i].vx = 0;
		rh_player_bullets[i].vy = 0;
	}
	for (int i = 0; i < ROAD_MAX_ENEMY_BULLETS; i++) {
		rh_enemy_bullets[i].active = false;
		rh_enemy_bullets[i].type = RH_POWER_NONE;
		rh_enemy_bullets[i].vx = 0;
		rh_enemy_bullets[i].vy = 0;
	}
	for (int i = 0; i < ROAD_MAX_EXPLOSIONS; i++) {
		rh_explosions[i].active = false;
	}
}

void rh_game_explosion_spawn(int16_t x, uint8_t lane) {
	for (int i = 0; i < ROAD_MAX_EXPLOSIONS; i++) {
		if (!rh_explosions[i].active) {
			rh_explosions[i].active = true;
			rh_explosions[i].x = x;
			rh_explosions[i].lane = lane;
			rh_explosions[i].timer = 6;
			return;
		}
	}
}

void rh_game_explosion_update(void) {
	for (int i = 0; i < ROAD_MAX_EXPLOSIONS; i++) {
		if (!rh_explosions[i].active)
			continue;
		if (rh_explosions[i].timer > 0) {
			rh_explosions[i].timer--;
		}
		if (rh_explosions[i].timer == 0) {
			rh_explosions[i].active = false;
		}
	}
}

void rh_game_explosion_display(void) {
	for (int i = 0; i < ROAD_MAX_EXPLOSIONS; i++) {
		if (!rh_explosions[i].active)
			continue;
		int y = rh_lane_to_y(rh_explosions[i].lane) + 4;
		int size = 1 + (rh_explosions[i].timer / 2);
		view_render.drawCircle(rh_explosions[i].x, y, size, WHITE);
		view_render.drawLine(rh_explosions[i].x - size, y, rh_explosions[i].x + size, y, WHITE);
		view_render.drawLine(rh_explosions[i].x, y - size, rh_explosions[i].x, y + size, WHITE);
		view_render.drawLine(rh_explosions[i].x - size, y - size, rh_explosions[i].x + size, y + size, WHITE);
		view_render.drawLine(rh_explosions[i].x - size, y + size, rh_explosions[i].x + size, y - size, WHITE);
	}
}

void rh_game_laser_update(void) {
	if (rh_active_power != RH_POWER_LASER)
		return;
	int16_t curx = ROAD_PLAYER_X + ROAD_PLAYER_W + 2;
	for (int lane = 0; lane < ROAD_LANE_COUNT; lane++) {
		int16_t scan_x = curx;
		while (true) {
			int16_t best_x = 10000;
			int best_type = -1; /* 0=en,1=drone,2=obstacle */
			int best_i = -1;
			/* find nearest enemy */
			for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
				if (!rh_enemies[i].active) continue;
				if (rh_enemies[i].lane != lane) continue;
				if (rh_enemies[i].x >= scan_x && rh_enemies[i].x < best_x) {
					best_x = rh_enemies[i].x;
					best_type = 0; best_i = i;
				}
			}
			/* find nearest drone */
			/* drones removed */
			/* find nearest obstacle */
			for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
				if (!rh_obstacles[i].active) continue;
				if (rh_obstacles[i].lane != lane) continue;
				if (rh_obstacles[i].x >= scan_x && rh_obstacles[i].x < best_x) {
					best_x = rh_obstacles[i].x;
					best_type = 2; best_i = i;
				}
			}
			if (best_type == -1)
				break;
			/* apply damage = 5 */
			if (best_type == 0) {
				rh_enemies[best_i].hp -= 5;
				if (rh_enemies[best_i].hp <= 0) {
					rh_score += rh_enemy_score_value(&rh_enemies[best_i]);
					rh_enemies[best_i].active = false;
					rh_game_explosion_spawn(rh_enemies[best_i].x + 4, rh_enemies[best_i].lane);
					scan_x = rh_enemies[best_i].x + 8;
					continue;
				}
				else {
					break;
				}
			}
			/* drones removed */
			else if (best_type == 2) {
				/* obstacles have hp field */
				if (rh_obstacles[best_i].hp > 0) {
					rh_obstacles[best_i].hp -= 5;
					if (rh_obstacles[best_i].hp <= 0) {
						rh_score += rh_obstacle_score_value(&rh_obstacles[best_i]);
						rh_obstacles[best_i].active = false;
						rh_game_explosion_spawn(rh_obstacles[best_i].x + 4, rh_obstacles[best_i].lane);
						scan_x = rh_obstacles[best_i].x + 8;
						continue;
					}
					else {
						break;
					}
				}
				else {
					/* instant destroy non-hp obstacles */
					rh_score += rh_obstacle_score_value(&rh_obstacles[best_i]);
					rh_obstacles[best_i].active = false;
					rh_game_explosion_spawn(rh_obstacles[best_i].x + 4, rh_obstacles[best_i].lane);
					scan_x = rh_obstacles[best_i].x + 8;
					continue;
				}
			}
		}
	}
}

void rh_game_laser_display(void) {
	if (rh_active_power != RH_POWER_LASER)
		return;
	int16_t curx = ROAD_PLAYER_X + ROAD_PLAYER_W + 2;
	for (int lane = 0; lane < ROAD_LANE_COUNT; lane++) {
		int16_t scan_x = curx;
		int16_t starty = rh_lane_to_y(lane) + 4;
		while (true) {
			int16_t best_x = 10000;
			int best_type = -1; int best_i = -1;
			for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
				if (!rh_enemies[i].active) continue;
				if (rh_enemies[i].lane != lane) continue;
				if (rh_enemies[i].x >= scan_x && rh_enemies[i].x < best_x) { best_x = rh_enemies[i].x; best_type = 0; best_i = i; }
			}
			/* drones removed */
			for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
				if (!rh_obstacles[i].active) continue;
				if (rh_obstacles[i].lane != lane) continue;
				if (rh_obstacles[i].x >= scan_x && rh_obstacles[i].x < best_x) { best_x = rh_obstacles[i].x; best_type = 2; best_i = i; }
			}
			if (best_type == -1) {
				view_render.drawLine(scan_x, starty, 127, starty, WHITE);
				break;
			}
			int16_t tx = best_x + 4;
			view_render.drawLine(scan_x, starty, tx, starty, WHITE);
			bool destroyed = false;
			if (best_type == 0) destroyed = (rh_enemies[best_i].hp <= 0);
			else if (best_type == 2) destroyed = (rh_obstacles[best_i].hp <= 0);
			if (destroyed) {
				scan_x = best_x + 8;
				continue;
			}
			else break;
		}
	}
}

void rh_game_bullet_explode(int bullet_index) {
	for (int k = 0; k < ROAD_MAX_ENEMIES; k++) {
		if (!rh_enemies[k].active)
			continue;
		if (rh_enemies[k].lane >= rh_player_bullets[bullet_index].lane - 1 &&
			rh_enemies[k].lane <= rh_player_bullets[bullet_index].lane + 1 &&
			rh_enemies[k].x >= rh_player_bullets[bullet_index].x - 8 &&
			rh_enemies[k].x <= rh_player_bullets[bullet_index].x + 8) {
			int ew = rh_enemy_width(&rh_enemies[k]);
			rh_score += rh_enemy_score_value(&rh_enemies[k]);
			rh_enemies[k].active = false;
			rh_game_on_target_killed(rh_enemies[k].is_tank);
			rh_game_explosion_spawn(rh_enemies[k].x + (ew / 2), rh_enemies[k].lane);
		}
	}
	for (int k = 0; k < ROAD_MAX_OBSTACLES; k++) {
		if (rh_obstacles[k].active &&
			rh_obstacles[k].lane >= rh_player_bullets[bullet_index].lane - 1 &&
			rh_obstacles[k].lane <= rh_player_bullets[bullet_index].lane + 1 &&
			rh_obstacles[k].x >= rh_player_bullets[bullet_index].x - 8 &&
			rh_obstacles[k].x <= rh_player_bullets[bullet_index].x + 8) {
			rh_score += rh_obstacle_score_value(&rh_obstacles[k]);
			rh_obstacles[k].active = false;
			rh_game_explosion_spawn(rh_obstacles[k].x + 4, rh_obstacles[k].lane);
		}
	}
}

void rh_game_bullet_fire(void) {
	for (int j = 0; j < ROAD_MAX_PLAYER_BULLETS; j++) {
		if (!rh_player_bullets[j].active) {
			rh_player_bullets[j].active = true;
			rh_player_bullets[j].x = ROAD_PLAYER_X + ROAD_PLAYER_W + 2;
			rh_player_bullets[j].lane = rh_player_lane;
			uint8_t damage = 1;
			if (rh_active_power == RH_POWER_EXPLOSIVE) {
				damage = 5;
			}
			rh_player_bullets[j].damage = damage;
			rh_player_bullets[j].piercing = (rh_active_power == RH_POWER_PIERCING);
			rh_player_bullets[j].type = (rh_active_power == RH_POWER_PIERCING ||
				rh_active_power == RH_POWER_EXPLOSIVE ||
				rh_active_power == RH_POWER_SPIDER)
				? rh_active_power : RH_POWER_NONE;
			rh_player_bullets[j].is_enemy = false;
			rh_player_bullets[j].vx = 0;
			rh_player_bullets[j].vy = 0;
			break;
		}
	}
}

void rh_game_bullet_autoshoot(void) {
	/* Route autoshoot to mode-button handler so manual/hold-to-fire works.
	   This preserves existing call sites but defers actual shooting to
	   the mode input handler which reads the raw button state. */
	rh_game_bullet_handle_mode_input();
}

void rh_game_bullet_handle_mode_input(void) {
	/* Edge-triggered: fire once when Mode transitions from unpressed->pressed.
	   This disables continuous auto-fire; holding Mode will not repeat shots. */
	/* If laser active, don't fire bullets. */
	if (rh_active_power == RH_POWER_LASER)
		return;

	/* Read raw mode button state (0 = pressed, 1 = released).
	   This button is pulled-up, so invert the value for logical pressed check.
	   When button released, reset shoot timer so next press fires immediately. */
	uint8_t mode_pressed = (io_button_mode_read() == BUTTON_HW_STATE_PRESSED);

	int base_interval = (rh_active_power == RH_POWER_GATLING) ? 1 : 3;
	int shoot_interval = base_interval;

	if (mode_pressed) {
		rh_shoot_timer++;
		if (rh_shoot_timer >= shoot_interval) {
			rh_shoot_timer = 0;
			rh_game_bullet_fire();
		}
	}
	else {
		/* Not pressed: reset timer to allow immediate fire on next press. */
		rh_shoot_timer = shoot_interval;
	}
}

bool rh_game_bullet_update(void) {
	for (int i = 0; i < ROAD_MAX_ENEMY_BULLETS; i++) {
		if (!rh_enemy_bullets[i].active)
			continue;

		int enemy_speed;
		if (rh_enemy_bullets[i].type == RH_POWER_EXPLOSIVE) {
			/* tank rockets: travel extremely fast toward player */
			int player_base = ROAD_BULLET_SPEED + ((rh_active_power == RH_POWER_GATLING) ? 2 : 0);
			enemy_speed = player_base + 5; /* much faster than player bullets */
			if (enemy_speed < 1) enemy_speed = 1;
		}
		else {
			enemy_speed = ROAD_BULLET_SPEED - 1;
			if (enemy_speed < 1) enemy_speed = 1;
		}
		rh_enemy_bullets[i].x -= enemy_speed;
		if (rh_enemy_bullets[i].x < -4) {
			rh_enemy_bullets[i].active = false;
			continue;
		}
		if (rh_enemy_bullets[i].lane == rh_player_lane &&
			rh_enemy_bullets[i].x <= ROAD_PLAYER_X + ROAD_PLAYER_W &&
			rh_enemy_bullets[i].x + 4 >= ROAD_PLAYER_X) {
			if (rh_empower_active) {
				rh_enemy_bullets[i].active = false;
			}
			else if (rh_game_consume_shield()) {
				rh_enemy_bullets[i].active = false;
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
			}
			else {
				rh_game_trigger_over();
				return true;
			}
		}
	}

	for (int i = 0; i < ROAD_MAX_PLAYER_BULLETS; i++) {
		if (!rh_player_bullets[i].active)
			continue;

		int bullet_speed = (rh_player_bullets[i].vx != 0) ? rh_player_bullets[i].vx : (ROAD_BULLET_SPEED + ((rh_active_power == RH_POWER_GATLING) ? 2 : 0));
		rh_player_bullets[i].x += bullet_speed;
		if (rh_player_bullets[i].vy != 0) {
			int lane = (int)rh_player_bullets[i].lane + rh_player_bullets[i].vy;
			if (lane < 0) {
				lane = 0;
			}
			else if (lane > ROAD_LANE_COUNT - 1) {
				lane = ROAD_LANE_COUNT - 1;
			}
			rh_player_bullets[i].lane = lane;
		}
		if (rh_player_bullets[i].x > 132) {
			rh_player_bullets[i].active = false;
			continue;
		}

		/* Player bullets that hit an enemy bullet (e.g. tank rockets) mutually
		   destroy each other and spawn a small explosion at the impact point. */
		for (int j = 0; j < ROAD_MAX_ENEMY_BULLETS; j++) {
			if (!rh_enemy_bullets[j].active || rh_enemy_bullets[j].lane != rh_player_bullets[i].lane)
				continue;
			if (rh_player_bullets[i].x + 3 >= rh_enemy_bullets[j].x && rh_player_bullets[i].x <= rh_enemy_bullets[j].x + 3) {
				rh_enemy_bullets[j].active = false;
				rh_player_bullets[i].active = false;
				rh_game_explosion_spawn((rh_player_bullets[i].x + rh_enemy_bullets[j].x) / 2, rh_player_bullets[i].lane);
				break;
			}
		}
		if (!rh_player_bullets[i].active)
			continue;

		/* drone collisions removed */
		if (!rh_player_bullets[i].active)
			continue;

		bool hit_enemy = false;
		for (int j = 0; j < ROAD_MAX_ENEMIES; j++) {
			if (!rh_enemies[j].active)
				continue;
			if (rh_enemies[j].lane != rh_player_bullets[i].lane)
				continue;
			int ew = rh_enemy_width(&rh_enemies[j]);
			/* count a hit anywhere the bullet box actually overlaps the
			   enemy's box -- clipping an edge is enough, no need to land
			   near the center -- but require real contact: no gap allowed
			   between the two boxes */
			if (rh_player_bullets[i].x + 3 >= rh_enemies[j].x && rh_player_bullets[i].x <= rh_enemies[j].x + ew) {
				if (!rh_player_bullets[i].piercing)
					rh_player_bullets[i].active = false;
				/* instant-kill for explosive bullets */
				if (rh_player_bullets[i].type == RH_POWER_EXPLOSIVE) {
					rh_score += rh_enemy_score_value(&rh_enemies[j]);
					rh_enemies[j].active = false;
					rh_game_on_target_killed(rh_enemies[j].is_tank);
					rh_game_explosion_spawn(rh_enemies[j].x + (ew / 2), rh_enemies[j].lane);
					if (rh_player_bullets[i].type == RH_POWER_EXPLOSIVE) {
						rh_game_bullet_explode(i);
					}
					if (rh_player_bullets[i].type == RH_POWER_SPIDER) {
						rh_enemies[j].trapped = true;
						rh_enemies[j].trap_timer = 10;
					}
				}
				else {
					rh_enemies[j].hp -= rh_player_bullets[i].damage;
					if (rh_enemies[j].hp <= 0) {
						rh_score += rh_enemy_score_value(&rh_enemies[j]);
						rh_enemies[j].active = false;
						rh_game_on_target_killed(rh_enemies[j].is_tank);
						rh_game_explosion_spawn(rh_enemies[j].x + (ew / 2), rh_enemies[j].lane);
					}
					if (rh_player_bullets[i].type == RH_POWER_EXPLOSIVE) {
						rh_game_bullet_explode(i);
					}
					if (rh_player_bullets[i].type == RH_POWER_SPIDER) {
						rh_enemies[j].trapped = true;
						rh_enemies[j].trap_timer = 10;
					}
				}
				hit_enemy = true;
				break;
			}
		}
		if (hit_enemy && !rh_player_bullets[i].active)
			continue;

		for (int j = 0; j < ROAD_MAX_OBSTACLES; j++) {
			if (!rh_obstacles[j].active)
				continue;
			if (rh_obstacles[j].lane != rh_player_bullets[i].lane)
				continue;
			int obs_w = RH_OBSTACLE_W_FENCE;
			if (rh_obstacles[j].type == 2) obs_w = 16;
			else if (rh_obstacles[j].type == 1 || rh_obstacles[j].type == 0) obs_w = 8;
			else if (rh_obstacles[j].type == 4) obs_w = 10;
			else if (rh_obstacles[j].type == 5) obs_w = RH_OBSTACLE_W_TRUCK;

			int obs_center = rh_obstacles[j].x + (obs_w / 2);
			/* same rule as enemies: any real overlap between the bullet box
			   and the obstacle box counts, edge included, but no gap */
			if (rh_player_bullets[i].x + 3 >= rh_obstacles[j].x && rh_player_bullets[i].x <= rh_obstacles[j].x + obs_w) {
				bool is_barrier = (rh_obstacles[j].type == 0 || rh_obstacles[j].type == 3);
				bool is_vehicle = (rh_obstacles[j].type == 4 || rh_obstacles[j].type == 5);
				bool is_hard = (rh_obstacles[j].type == 0 || rh_obstacles[j].type == 3);
				/* only explosive bullets can destroy rock/barrier obstacles */
				if (rh_player_bullets[i].type == RH_POWER_EXPLOSIVE) {
					rh_score += rh_obstacle_score_value(&rh_obstacles[j]);
					rh_obstacles[j].active = false;
					rh_game_explosion_spawn(obs_center, rh_obstacles[j].lane);
					rh_game_bullet_explode(i);
					if (!rh_player_bullets[i].piercing) {
						rh_player_bullets[i].active = false;
					}
				}
				else if (is_barrier) {
					rh_player_bullets[i].active = false;
				}
				else if (is_vehicle) {
					if (!rh_player_bullets[i].piercing) {
						rh_player_bullets[i].active = false;
					}
					if (rh_obstacles[j].hp > 0) {
						rh_obstacles[j].hp -= rh_player_bullets[i].damage;
					}
					if (rh_obstacles[j].hp <= 0) {
						rh_score += rh_obstacle_score_value(&rh_obstacles[j]);
						rh_obstacles[j].active = false;
						rh_game_explosion_spawn(obs_center, rh_obstacles[j].lane);
					}
				}
				else if (!is_hard) {
					rh_obstacles[j].active = false;
					rh_game_explosion_spawn(obs_center, rh_obstacles[j].lane);
					if (!rh_player_bullets[i].piercing) {
						rh_player_bullets[i].active = false;
					}
				}
				else {
					rh_player_bullets[i].active = false;
				}
				break;
			}
		}
	}
	return false;
}