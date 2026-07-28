#include "scr_road_hunter.h"


static int rh_text_len(const char *s) {
	int n = 0;
	while (*s++) {
		n++;
	}
	return n;
}


static void rh_draw_ripple(int cx, int cy, int phase) {
	int step = (rh_game_tick + phase) % 12;
	int r1	 = 1 + (step / 4);
	int r2	 = r1 + 3;
	view_render.drawPixel(cx - r1, cy, WHITE);
	view_render.drawPixel(cx + r1, cy, WHITE);
	view_render.drawPixel(cx - r2, cy + 1, WHITE);
	view_render.drawPixel(cx + r2, cy + 1, WHITE);
	if (step < 8) {
		view_render.drawPixel(cx - r1 - 2, cy - 1, WHITE);
		view_render.drawPixel(cx + r1 + 2, cy - 1, WHITE);
	}
}


static void rh_game_player_thruster_display(int py) {
	int by = py + (ROAD_PLAYER_H / 2) - 1;
	int bx = ROAD_PLAYER_X - 1;
	int flicker = rh_game_tick & 0x03;
	int flame_len = 3 + (flicker < 2 ? 1 : 0);

	/* bright flame core, right behind the exhaust */
	view_render.fillTriangle(bx, by - 1, bx, by + 2, bx - flame_len, by, WHITE);

	/* smoke puffs: dense near the car, thinning out with distance */
	int p0x = bx - flame_len - 2 - (rh_game_tick % 3);
	view_render.fillRect(p0x, by - 1, 2, 2, WHITE);

	int p1x = bx - flame_len - 6 - (rh_game_tick % 3);
	int p1y = by + ((rh_game_tick / 2) % 3) - 1;
	view_render.drawPixel(p1x, p1y, WHITE);
	view_render.drawPixel(p1x + 1, p1y + 1, WHITE);

	int p2x = bx - flame_len - 10 - (rh_game_tick % 4);
	int p2y = by + ((rh_game_tick / 3) % 3) - 1;
	if ((rh_game_tick & 1) == 0) {
		view_render.drawPixel(p2x, p2y, WHITE);
	}
}


static void rh_game_player_empower_aura(int py) {
	int cx = ROAD_PLAYER_X + (ROAD_PLAYER_W / 2);
	int cy = py + (ROAD_PLAYER_H / 2) - 1;
	int flicker = rh_game_tick & 0x03;

	/* pulsing outer ring */
	view_render.drawCircle(cx, cy, 9 + (flicker < 2 ? 1 : 0), WHITE);

	/* spikes bursting upward off the roofline */
	for (int k = -1; k <= 1; k++) {
		int sx = cx + k * 5;
		int spike_h = 3 + ((rh_game_tick + k * 3) % 3);
		int top = py - 3;
		view_render.drawLine(sx, top, sx - 1, top - spike_h, WHITE);
		view_render.drawLine(sx, top, sx + 1, top - spike_h, WHITE);
	}
	/* spikes bursting downward off the undercarriage */
	for (int k = -1; k <= 1; k++) {
		int sx = cx + k * 5;
		int spike_h = 3 + ((rh_game_tick + k * 2) % 3);
		int bottom = py + ROAD_PLAYER_H + 2;
		view_render.drawLine(sx, bottom, sx - 1, bottom + spike_h, WHITE);
		view_render.drawLine(sx, bottom, sx + 1, bottom + spike_h, WHITE);
	}
	/* stray energy sparks drifting off the back */
	int trail_x = ROAD_PLAYER_X - 5 - (rh_game_tick % 4);
	view_render.drawPixel(trail_x, py, WHITE);
	view_render.drawPixel(trail_x - 2, py + ROAD_PLAYER_H, WHITE);
}


static void rh_game_player_boat_hull_display(int py) {
	int bx = ROAD_PLAYER_X;
	int by = py + 1;			  /* deck line */
	int bb = py + ROAD_PLAYER_H; /* hull bottom / waterline */

	view_render.fillRect(bx, by, ROAD_PLAYER_W - 4, bb - by, WHITE);
	view_render.fillTriangle(bx + ROAD_PLAYER_W - 4, by, bx + ROAD_PLAYER_W + 2, (by + bb) / 2,
		bx + ROAD_PLAYER_W - 4, bb, WHITE);
	view_render.fillRect(bx - 1, by + 1, 2, bb - by - 2, WHITE); /* transom stern */

	/* small pilot cabin amidships with a window slit */
	view_render.fillRect(bx + 3, by - 4, 6, 4, WHITE);
	view_render.drawFastHLine(bx + 4, by - 3, 4, BLACK);

	/* waterline ripples lapping at the hull so it reads as floating on
	   real, animated water instead of a flat static line */
	rh_draw_ripple(bx - 1, bb + 2, 0);
	rh_draw_ripple(bx + 9, bb + 2, 6);
}


static void rh_game_player_boss_form_display(int py) {
	if (rh_boss.type == RH_BOSS_SEA) {
		int deck_x = ROAD_PLAYER_X + (ROAD_PLAYER_W / 2) - 3;
		int deck_y = py - 3; /* top of the pilot cabin */
		view_render.drawFastVLine(deck_x + 1, deck_y - 4, 4, WHITE);				 /* mast */
		int flap = (rh_game_tick / 4) % 2;
		view_render.drawLine(deck_x + 2, deck_y - 4, deck_x + 4 + flap, deck_y - 3, WHITE); /* flag */
	}
	else if (rh_boss.type == RH_BOSS_SKY) {
		int cy = py + (ROAD_PLAYER_H / 2);
		view_render.fillTriangle(ROAD_PLAYER_X + 3, cy, ROAD_PLAYER_X - 5, cy - 6, ROAD_PLAYER_X + 7, cy - 2, WHITE);
		view_render.fillTriangle(ROAD_PLAYER_X + 3, cy, ROAD_PLAYER_X - 5, cy + 6, ROAD_PLAYER_X + 7, cy + 2, WHITE);
	}
}

static void rh_game_player_display(void) {
	int py = rh_lane_to_y(rh_player_lane);

	rh_game_player_thruster_display(py);

	bool is_boat = rh_game_boss_active() && rh_boss.type == RH_BOSS_SEA;
	if (is_boat) {
		rh_game_player_boat_hull_display(py);
	}
	else {
		view_render.drawBitmap(ROAD_PLAYER_X, py - 2, bitmap_car, CAR_SPRITE_W, CAR_SPRITE_H, WHITE);
	}

	/* heavy cannon mounted on the bow, muzzle brake at the tip */
	int gx = ROAD_PLAYER_X + ROAD_PLAYER_W;
	int gy = py + (ROAD_PLAYER_H / 2) - 2;
	view_render.drawFastHLine(gx, gy, 5, WHITE);
	view_render.drawFastHLine(gx, gy + 1, 5, WHITE);
	view_render.fillRect(gx + 4, gy - 1, 2, 4, WHITE);

	if (rh_shield_active || rh_active_power == RH_POWER_SHIELD) {
		view_render.drawRect(ROAD_PLAYER_X - 2, py - 3, ROAD_PLAYER_W + 4, ROAD_PLAYER_H + 5, WHITE);
	}

	if (rh_empower_active) {
		rh_game_player_empower_aura(py);
	}

	if (rh_game_boss_active()) {
		rh_game_player_boss_form_display(py);
	}
}

static void rh_game_enemy_display(void) {
	for (int i = 0; i < ROAD_MAX_ENEMIES; i++) {
		if (!rh_enemies[i].active)
			continue;
		int tx = rh_enemies[i].x;
		int ty = rh_lane_to_y(rh_enemies[i].lane);
		int ew = rh_enemy_width(&rh_enemies[i]);
		if (rh_enemies[i].is_tank) {
			/* long hull */
			view_render.fillRect(tx + 6, ty, RH_TANK_W - 6, 8, WHITE);
			/* sloped front glacis armor */
			view_render.fillRect(tx + 6, ty - 1, 3, 2, WHITE);
			/* tread marks along the bottom of the hull */
			for (int dx = 0; dx < RH_TANK_W - 7; dx += 3) {
				view_render.drawFastHLine(tx + 7 + dx, ty + 8, 2, WHITE);
			}
			/* turret, set back from the front for a beefier silhouette */
			view_render.fillRect(tx + 11, ty - 3, 8, 4, WHITE);
			/* thick double-barrel gun, pointing toward the player (left) */
			view_render.drawFastHLine(tx - 1, ty - 2, 9, WHITE);
			view_render.drawFastHLine(tx - 1, ty - 1, 9, WHITE);
			/* muzzle brake at the tip */
			view_render.fillRect(tx - 2, ty - 3, 2, 3, WHITE);
			/* second hit point remaining is shown as a dot on the turret */
			if (rh_enemies[i].hp > 1) {
				view_render.drawPixel(tx + RH_TANK_W - 2, ty - 2, WHITE);
			}
		}
		else {			
			int bx = tx;
			int by = ty;
			view_render.fillRect(bx + 2, by + 3, 5, 3, WHITE);
			view_render.fillRect(bx + 5, by + 1, 3, 3, WHITE);
			view_render.fillTriangle(bx, by + 5, bx + 2, by + 3, bx + 2, by + 6, WHITE);
			view_render.fillRect(bx + 2, by + 6, 2, 1, WHITE);
			view_render.fillRect(bx + 6, by + 6, 2, 1, WHITE);
		}
		if (rh_enemies[i].trapped) {
			int x = tx + (ew / 2);
			int y = ty + 4;
			view_render.drawTriangle(x - 6, y + 4, x + 6, y + 4, x, y - 6, WHITE);
		}
	}
}

static void rh_game_obstacle_display(void) {
	for (int i = 0; i < ROAD_MAX_OBSTACLES; i++) {
		if (!rh_obstacles[i].active)
			continue;
		if (rh_obstacles[i].type == 0) {
			view_render.drawBitmap(rh_obstacles[i].x, rh_lane_to_y(rh_obstacles[i].lane), bitmap_obstacle, 8, 8, WHITE);
		}
		else {
			int y = rh_lane_to_y(rh_obstacles[i].lane) + 1;
			if (rh_obstacles[i].type == 1) {
				/* small fast hazard: a traffic cone (pointed top, flared base) */
				int cx = rh_obstacles[i].x;
				view_render.fillTriangle(cx + 4, y, cx, y + 5, cx + 8, y + 5, WHITE);
				view_render.fillRect(cx, y + 5, 8, 2, WHITE);
				view_render.drawFastHLine(cx + 2, y + 3, 4, BLACK);
			}
			else if (rh_obstacles[i].type == 2) {
				/* big slow hazard: a strapped cargo crate, clearly not just a plain block */
				int cx = rh_obstacles[i].x;
				view_render.drawRect(cx, y, 16, 7, WHITE);
				view_render.fillRect(cx + 1, y + 1, 14, 5, WHITE);
				view_render.drawLine(cx + 1, y + 1, cx + 14, y + 5, BLACK);
				view_render.drawLine(cx + 14, y + 1, cx + 1, y + 5, BLACK);
				view_render.drawFastVLine(cx + 8, y, 7, BLACK);
			}
			else if (rh_obstacles[i].type == 4) {
				/* car: body, windshield gap for a real cabin look, small wheels */
				int cx = rh_obstacles[i].x;
				view_render.fillRect(cx, y + 1, 10, 4, WHITE);
				view_render.fillRect(cx + 2, y, 3, 2, WHITE);
				view_render.fillRect(cx + 6, y, 2, 2, WHITE);
				view_render.fillRect(cx + 1, y + 5, 2, 1, WHITE);
				view_render.fillRect(cx + 7, y + 5, 2, 1, WHITE);
			}
			else if (rh_obstacles[i].type == 5) {
				int tx2 = rh_obstacles[i].x;
				int cab_w = 6;
				int trailer_w = RH_OBSTACLE_W_TRUCK - cab_w;
				view_render.fillRect(tx2, y - 1, cab_w, 6, WHITE);
				view_render.fillRect(tx2 + 1, y - 2, 3, 2, WHITE);
				view_render.fillRect(tx2 + cab_w, y, trailer_w, 5, WHITE);
				for (int px = tx2 + cab_w + 4; px < tx2 + RH_OBSTACLE_W_TRUCK - 2; px += 4) {
					view_render.drawFastVLine(px, y, 5, WHITE);
				}
				view_render.fillRect(tx2 + 2, y + 5, 2, 1, WHITE);
				for (int wx = tx2 + cab_w + 2; wx < tx2 + RH_OBSTACLE_W_TRUCK - 2; wx += 6) {
					view_render.fillRect(wx, y + 5, 2, 1, WHITE);
				}
			}
			else {
				/* fence: 3 posts, 3 rails and X cross-bracing, like a road barrier */
				int fx = rh_obstacles[i].x;
				int fy = y;
				int fw = RH_OBSTACLE_W_FENCE;
				view_render.drawFastVLine(fx, fy, 6, WHITE);
				view_render.drawFastVLine(fx + fw / 2, fy, 6, WHITE);
				view_render.drawFastVLine(fx + fw - 1, fy, 6, WHITE);
				view_render.drawFastHLine(fx, fy, fw, WHITE);
				view_render.drawFastHLine(fx, fy + 3, fw, WHITE);
				view_render.drawFastHLine(fx, fy + 5, fw, WHITE);
				view_render.drawLine(fx, fy, fx + fw / 2, fy + 5, WHITE);
				view_render.drawLine(fx + fw / 2, fy, fx, fy + 5, WHITE);
				view_render.drawLine(fx + fw / 2, fy, fx + fw - 1, fy + 5, WHITE);
				view_render.drawLine(fx + fw - 1, fy, fx + fw / 2, fy + 5, WHITE);
			}
		}
	}
}

static void rh_game_chest_display(void) {
	for (int i = 0; i < ROAD_MAX_CHESTS; i++) {
		if (!rh_chests[i].active)
			continue;
		int x = rh_chests[i].x;
		int y = rh_lane_to_y(rh_chests[i].lane);
		view_render.drawBitmap(x, y, bitmap_chest, 8, 8, WHITE);

		/* Blinking sparkle glints so the chest is easy to spot at a glance.
		   Two alternating phases (driven by rh_game_tick) give it a
		   twinkling look instead of a single static glint. */
		int phase = (rh_game_tick / 4) % 3;
		if (phase == 0) {
			/* small plus-shaped glint, top-right corner */
			view_render.drawPixel(x + 10, y - 3, WHITE);
			view_render.drawPixel(x + 9, y - 2, WHITE);
			view_render.drawPixel(x + 11, y - 2, WHITE);
			view_render.drawPixel(x + 10, y - 1, WHITE);
			/* small glint, bottom-left corner */
			view_render.drawPixel(x - 3, y + 10, WHITE);
			view_render.drawPixel(x - 2, y + 11, WHITE);
		}
		else if (phase == 1) {
			/* corner tick-marks framing the chest */
			view_render.drawFastHLine(x - 3, y - 2, 2, WHITE);
			view_render.drawFastVLine(x - 2, y - 3, 2, WHITE);
			view_render.drawFastHLine(x + 9, y + 9, 2, WHITE);
			view_render.drawFastVLine(x + 10, y + 8, 2, WHITE);
		}
		/* phase == 2: no glint, brief pause between sparkles for a
		   noticeable on/off blink instead of a constant shimmer */
	}
}

static void rh_game_bullet_display(void) {
	for (int i = 0; i < ROAD_MAX_ENEMY_BULLETS; i++) {
		if (!rh_enemy_bullets[i].active)
			continue;
		int y = rh_lane_to_y(rh_enemy_bullets[i].lane) + 3;
		if (rh_enemy_bullets[i].is_enemy && rh_enemy_bullets[i].type == RH_POWER_EXPLOSIVE) {
			/* draw short fast rocket */
			view_render.fillRect(rh_enemy_bullets[i].x, y, 3, 2, WHITE);
			view_render.drawLine(rh_enemy_bullets[i].x + 3, y - 1, rh_enemy_bullets[i].x + 4, y + 0, WHITE);
			view_render.drawLine(rh_enemy_bullets[i].x + 3, y + 2, rh_enemy_bullets[i].x + 4, y + 1, WHITE);
		}
		else {
			view_render.fillRect(rh_enemy_bullets[i].x, y, 3, 2, WHITE);
		}
	}
	for (int i = 0; i < ROAD_MAX_PLAYER_BULLETS; i++) {
		if (!rh_player_bullets[i].active)
			continue;
		int y = rh_lane_to_y(rh_player_bullets[i].lane) + 3;
		switch (rh_player_bullets[i].type) {
		case RH_POWER_EXPLOSIVE:
			view_render.fillCircle(rh_player_bullets[i].x + 1, y + 1, 2, WHITE);
			break;
		case RH_POWER_PIERCING:
			view_render.fillTriangle(rh_player_bullets[i].x, y,
				rh_player_bullets[i].x + 4, y + 1,
				rh_player_bullets[i].x, y + 3,
				WHITE);
			break;
		case RH_POWER_SPIDER:
			view_render.fillCircle(rh_player_bullets[i].x + 1, y + 1, 2, WHITE);
			view_render.drawCircle(rh_player_bullets[i].x + 1, y + 1, 3, WHITE);
			view_render.drawLine(rh_player_bullets[i].x + 1, y - 1, rh_player_bullets[i].x + 1, y + 5, WHITE);
			view_render.drawLine(rh_player_bullets[i].x - 1, y + 1, rh_player_bullets[i].x + 5, y + 1, WHITE);
			view_render.drawLine(rh_player_bullets[i].x - 1, y - 1, rh_player_bullets[i].x + 3, y + 3, WHITE);
			view_render.drawLine(rh_player_bullets[i].x + 3, y - 1, rh_player_bullets[i].x - 1, y + 3, WHITE);
			break;
		default:
			view_render.fillRect(rh_player_bullets[i].x, y, 4, 2, WHITE);
			break;
		}
	}
}

/* Drone display removed */


static void rh_game_empower_display(void) {
	int x = 2;
	int y = 55;

	if (rh_empower_active) {
		view_render.setCursor(x, y);
		if ((rh_game_tick / 3) & 1) {
			view_render.print("EMPOWERED!");
		}
		view_render.print(" ");
		view_render.print((int)(rh_empower_timer / 8));
		view_render.print("s");
		return;
	}

	int bar_w = RH_EMPOWER_TANKS_TO_FULL * 4 + 2;
	view_render.drawRect(x, y, bar_w, 5, WHITE);
	for (int i = 0; i < RH_EMPOWER_TANKS_TO_FULL; i++) {
		int fill_x = x + 2 + i * 4;
		if (i < rh_empower_charge) {
			view_render.fillRect(fill_x, y + 1, 3, 3, WHITE);
		}
	}
}


static void rh_game_road_backdrop_display(void) {
	if (rh_game_boss_active() && rh_boss.type == RH_BOSS_SEA) {
		for (int lane = 1; lane < ROAD_LANE_COUNT; lane++) {
			int y = rh_lane_to_y(lane) - 2;
			for (int x = -(rh_game_tick % 8); x < 128; x += 8) {
				int wy = y + (((x / 4) + rh_game_tick) % 2);
				view_render.drawFastHLine(x, wy, 3, WHITE);
			}
		}
	}
	else if (rh_game_boss_active() && rh_boss.type == RH_BOSS_SKY) {
		for (int x = -(rh_game_tick % 44); x < 128; x += 44) {
			view_render.drawFastHLine(x, 14, 6, WHITE);
			view_render.drawPixel(x + 8, 14, WHITE);
		}
		for (int x = -((rh_game_tick * 2) % 56); x < 128; x += 56) {
			view_render.drawFastHLine(x, 52, 5, WHITE);
		}
	}
	else {
		for (int lane = 1; lane < ROAD_LANE_COUNT; lane++) {
			int y = rh_lane_to_y(lane) - 2;
			for (int x = -(rh_game_tick % 20); x < 128; x += 20) {
				view_render.drawFastHLine(x, y, 10, WHITE);
			}
		}
	}
}

static void rh_game_boss_hp_bar_display(int16_t x0, int16_t w, int16_t y_top) {
	view_render.drawRect(x0, y_top - 4, w, 3, WHITE);
	if (rh_boss.max_hp > 0) {
		int fillw = ((w - 2) * rh_boss.hp) / rh_boss.max_hp;
		if (fillw > 0) {
			view_render.fillRect(x0 + 1, y_top - 3, fillw, 1, WHITE);
		}
	}
}

static void rh_game_boss_land_display(void) {
	int x0 = rh_boss.x;
	int top_lane = rh_boss.land_base_lane;
	int y_top = rh_lane_to_y(top_lane) - 3;
	int y_bot = rh_lane_to_y(top_lane + RH_BOSS_LANE_COUNT - 1) + 8;
	int w	  = RH_BOSS_LAND_W;

	
	view_render.fillRect(x0 + 4, y_top, w - 4, y_bot - y_top, WHITE);
	for (int dx = 8; dx < w - 4; dx += 5) {
		view_render.drawFastVLine(x0 + dx, y_top + 1, y_bot - y_top - 2, BLACK);
	}
	/* angular sloped nose plate jutting toward the player */
	view_render.fillTriangle(x0 + 4, y_top, x0 - 4, (y_top + y_bot) / 2, x0 + 4, y_bot, WHITE);
	/* rear armor skirt */
	view_render.fillRect(x0 + w - 4, y_top + 2, 4, y_bot - y_top - 4, WHITE);

	
	int tread_off = rh_game_tick % 4;
	for (int dx = -tread_off; dx < w - 6; dx += 4) {
		view_render.drawFastVLine(x0 + 6 + dx, y_bot - 2, 2, BLACK);
	}

	
	for (int slot = 0; slot < RH_BOSS_LANE_COUNT; slot++) {
		int ly = rh_lane_to_y(top_lane + slot) + 3;
		view_render.fillRect(x0 - 10, ly - 1, 14, 3, WHITE);
		view_render.fillRect(x0 - 12, ly - 2, 3, 5, WHITE);
	}

	
	for (int slot = 0; slot < RH_BOSS_LANE_COUNT; slot++) {
		int ly = rh_lane_to_y(top_lane + slot);
		if (slot == rh_boss.weak_slot) {
			if ((rh_game_tick / 2) & 1) {
				view_render.fillRect(x0 + 8, ly, w - 14, 6, BLACK);
			}
			view_render.drawRect(x0 + 7, ly - 1, w - 12, 8, BLACK);
		}
		else {
			view_render.drawFastHLine(x0 + 8, ly + 3, w - 14, BLACK);
		}
	}
	rh_game_boss_hp_bar_display(x0, w, y_top);
}

static void rh_game_boss_sea_display(void) {
	int x0 = rh_boss.x;
	int y_top = rh_lane_to_y(RH_BOSS_SEA_LANE_LO) - 2;
	int y_bot = rh_lane_to_y(RH_BOSS_SEA_LANE_LO + RH_BOSS_SEA_LANE_COUNT - 1) + 8;
	int w	  = RH_BOSS_SEA_W;
	int mid_y = (y_top + y_bot) / 2;


	view_render.fillRect(x0 + 4, y_top + 2, w - 4, y_bot - y_top - 4, WHITE);
	view_render.fillTriangle(x0 + 4, y_top + 1, x0 - 7, mid_y, x0 + 4, y_bot - 1, WHITE);
	for (int dx = 10; dx < w - 4; dx += 9) {
		view_render.drawFastVLine(x0 + dx, y_top + 3, y_bot - y_top - 6, BLACK);
	}
	view_render.drawFastHLine(x0 + 4, y_top + 2, w - 4, BLACK);
	view_render.drawFastHLine(x0 + 4, y_bot - 3, w - 4, BLACK);

	/* tall blocky conning tower topped with a radar dish, taller and
	   heavier than before */
	view_render.fillRect(x0 + (w / 2) - 6, mid_y - 11, 12, 9, WHITE);
	view_render.fillRect(x0 + (w / 2) - 4, mid_y - 15, 8, 5, WHITE);
	view_render.drawFastVLine(x0 + (w / 2), mid_y - 19, 4, WHITE);
	view_render.drawCircle(x0 + (w / 2), mid_y - 20, 2, WHITE);

	
	for (int lane = 0; lane < ROAD_LANE_COUNT; lane++) {
		int ly = rh_lane_to_y(lane) + 3;
		view_render.fillRect(x0 - 3, ly - 3, 5, 6, WHITE);	  /* mounting cradle on the hull */
		view_render.fillRect(x0 - 13, ly - 2, 10, 4, WHITE); /* short, wide launch tube */
		view_render.drawCircle(x0 - 13, ly, 2, WHITE);		  /* open tube mouth */
		view_render.drawPixel(x0 - 13, ly, BLACK);			  /* dark bore hole */
	}

	
	for (int lane = 0; lane < ROAD_LANE_COUNT; lane++) {
		int ly = rh_lane_to_y(lane);
		if (lane == rh_boss.weak_slot) {
			view_render.drawRect(x0 + 7, ly - 1, w - 12, 8, BLACK);
			if ((rh_game_tick / 2) & 1) {
				view_render.fillRect(x0 + 8, ly, w - 14, 6, BLACK);
			}
		}
		else {
			view_render.drawFastHLine(x0 + 8, ly + 3, w - 14, BLACK);
		}
	}

	/* fore and aft deck domes, drawn last so they sit on top of the seams */
	view_render.fillCircle(x0 + 12, mid_y, 4, WHITE);
	view_render.fillCircle(x0 + w - 13, mid_y, 4, WHITE);

	rh_game_boss_hp_bar_display(x0, w, y_top);
}

static void rh_game_boss_sky_display(void) {
	int x0 = rh_boss.x;
	int top_lane = rh_boss.sky_base_lane;
	int mid_lane = rh_boss.sky_base_lane + 1;
	int bot_lane = rh_boss.sky_base_lane + (RH_BOSS_LANE_COUNT - 1);
	int w		 = RH_BOSS_SKY_W;

	
	int fy = rh_lane_to_y(mid_lane) + 1;
	view_render.fillRect(x0 + 8, fy, w - 8, 7, WHITE);
	view_render.fillTriangle(x0, fy + 3, x0 + 8, fy, x0 + 8, fy + 7, WHITE);
	view_render.fillRect(x0 + w - 14, fy - 3, 8, 4, WHITE);   /* cockpit hump */
	view_render.drawFastHLine(x0 + w - 12, fy - 2, 4, BLACK); /* canopy glass */
	view_render.fillTriangle(x0 + w - 2, fy - 4, x0 + w + 3, fy - 1, x0 + w - 2, fy + 2, WHITE);
	view_render.fillTriangle(x0 + w - 2, fy + 5, x0 + w + 3, fy + 8, x0 + w - 2, fy + 11, WHITE);

	
	int ty = rh_lane_to_y(top_lane) + 1;
	int by = rh_lane_to_y(bot_lane) + 1;
	if (rh_boss.wing_lit) {
		view_render.fillTriangle(x0 + 2, ty + 7, x0 + w - 8, ty, x0 + w - 8, ty + 7, WHITE);
		view_render.fillTriangle(x0 + 2, by, x0 + w - 8, by, x0 + w - 8, by + 7, WHITE);
	}
	else {
		view_render.drawTriangle(x0 + 2, ty + 7, x0 + w - 8, ty, x0 + w - 8, ty + 7, WHITE);
		view_render.drawTriangle(x0 + 2, by, x0 + w - 8, by, x0 + w - 8, by + 7, WHITE);
	}
	
	view_render.fillRect(x0 + w - 8, ty, 6, 7, WHITE);
	view_render.fillRect(x0 + w - 8, by, 6, 7, WHITE);

	/* struts linking the wings to the fuselage */
	view_render.drawFastVLine(x0 + w - 7, ty + 7, fy - (ty + 7), WHITE);
	view_render.drawFastVLine(x0 + w - 7, fy + 7, by - (fy + 7), WHITE);

	rh_game_boss_hp_bar_display(x0, w, rh_lane_to_y(top_lane) - 2);
}

static void rh_game_boss_projectile_display(void) {
	for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
		if (!rh_boss_projectiles[i].active)
			continue;
		int x = rh_boss_projectiles[i].x;
		int y = rh_lane_to_y(rh_boss_projectiles[i].lane) + 3;
		switch (rh_boss_projectiles[i].type) {
		case RH_BPROJ_LAND_SHELL:
			view_render.fillCircle(x, y, 3, WHITE);
			break;
		case RH_BPROJ_MINE: {
			
			int my = y + 2;
			view_render.fillRect(x - 3, my, 7, 2, WHITE);
			view_render.drawLine(x - 3, my, x, my - 3, WHITE);
			view_render.drawLine(x + 3, my, x, my - 3, WHITE);
			if ((rh_game_tick / 3) & 1) {
				view_render.drawPixel(x, my - 4, WHITE);
			}
			break;
		}
		case RH_BPROJ_TORPEDO:
			view_render.fillRect(x, y - 1, 6, 2, WHITE);
			view_render.drawPixel(x + 6, y, WHITE);
			break;
		case RH_BPROJ_MISSILE:
			view_render.fillRect(x, y - 1, 5, 2, WHITE);
			view_render.drawLine(x + 5, y - 1, x + 7, y, WHITE);
			view_render.drawLine(x + 5, y + 2, x + 7, y, WHITE);
			break;
		default: break;
		}
	}
}

static void rh_game_boss_display(void) {
	if (rh_boss.phase == RH_BOSS_PHASE_NONE)
		return;

	switch (rh_boss.type) {
	case RH_BOSS_LAND: rh_game_boss_land_display(); break;
	case RH_BOSS_SEA:  rh_game_boss_sea_display(); break;
	case RH_BOSS_SKY:  rh_game_boss_sky_display(); break;
	default: break;
	}

	if (rh_boss.phase == RH_BOSS_PHASE_FIGHT) {
		rh_game_boss_projectile_display();
	}

	const char *name;
	switch (rh_boss.type) {
	case RH_BOSS_LAND: name = "TANK BOSS"; break;
	case RH_BOSS_SEA:  name = "WARSHIP BOSS"; break;
	case RH_BOSS_SKY:  name = "AIRCRAFT BOSS"; break;
	default:		   name = ""; break;
	}
	/* bottom strip, right-aligned: the boss sprite patrols the whole
	   playfield now, so this is the only spot guaranteed to stay clear
	   of it, and it no longer shares space with the power HUD (moved up
	   next to Score) */
	view_render.setCursor(ROAD_SCREEN_WIDTH - (rh_text_len(name) * 6) - 2, 55);
	view_render.print(name);
}

static void rh_game_frame_display(void) {
	view_render.drawFastHLine(0, ROAD_TOP_BORDER_Y, 128, WHITE);
	view_render.drawFastHLine(0, ROAD_BOTTOM_BORDER_Y, 128, WHITE);

	rh_game_road_backdrop_display();

	rh_game_empower_display();
	rh_game_player_display();

	if (rh_game_boss_active()) {
		/* boss fights are boss + player only: no enemies/obstacles/chests */
		rh_game_boss_display();
	}
	else {
		rh_game_enemy_display();
		rh_game_obstacle_display();
		rh_game_chest_display();
	}

	rh_game_bullet_display();
	/* draw laser beam if active */
	rh_game_laser_display();
	rh_game_explosion_display();
}


#define RH_GAME_OVER_LINE_W        (4 * 18) /* 4 chars, textSize 3 => 18px/char */
#define RH_GAME_OVER_LINE_H        (24)     /* textSize 3 => 8px * 3            */
#define RH_GAME_OVER_LINE_GAP      (2)
#define RH_GAME_OVER_LINE0_Y       (8)
#define RH_GAME_OVER_LINE1_Y       (RH_GAME_OVER_LINE0_Y + RH_GAME_OVER_LINE_H + RH_GAME_OVER_LINE_GAP)
#define RH_GAME_OVER_START_Y       (-RH_GAME_OVER_LINE_H - 4) /* fully off-screen above */
#define RH_GAME_OVER_LINE1_DELAY   (4)      /* ticks after line0 starts before line1 starts falling */


static int16_t rh_game_over_line_y(int16_t target_y, uint16_t elapsed) {
	if (elapsed >= RH_GAME_OVER_ANIM_FALL_TICKS) {
		return target_y;
	}
	int32_t t_norm = ((int32_t)elapsed * 100) / RH_GAME_OVER_ANIM_FALL_TICKS;		 /* 0..100 */
	int32_t eased  = 100 - (((100 - t_norm) * (100 - t_norm)) / 100);				 /* ease-out */
	return (int16_t)(RH_GAME_OVER_START_Y + (((target_y - RH_GAME_OVER_START_Y) * eased) / 100));
}

static void rh_game_over_intro_display() {
	uint16_t elapsed = RH_GAME_OVER_ANIM_TOTAL_TICKS - rh_state_timer;

	uint16_t elapsed0 = elapsed;
	uint16_t elapsed1 = (elapsed > RH_GAME_OVER_LINE1_DELAY) ? (elapsed - RH_GAME_OVER_LINE1_DELAY) : 0;

	int16_t y0 = rh_game_over_line_y(RH_GAME_OVER_LINE0_Y, elapsed0);
	int16_t y1 = rh_game_over_line_y(RH_GAME_OVER_LINE1_Y, elapsed1);

	
	int16_t x0_shake = (elapsed0 == RH_GAME_OVER_ANIM_FALL_TICKS) ? -1 :
						(elapsed0 == RH_GAME_OVER_ANIM_FALL_TICKS + 1) ? 1 : 0;
	int16_t x1_shake = (elapsed1 == RH_GAME_OVER_ANIM_FALL_TICKS) ? -1 :
						(elapsed1 == RH_GAME_OVER_ANIM_FALL_TICKS + 1) ? 1 : 0;

	int16_t x = (ROAD_SCREEN_WIDTH - RH_GAME_OVER_LINE_W) / 2;

	view_render.setTextSize(3);
	view_render.setTextColor(WHITE);
	view_render.setCursor(x + x0_shake, y0);
	view_render.print("GAME");
	view_render.setCursor(x + x1_shake, y1);
	view_render.print("OVER");
}

static void view_scr_road_hunter();

view_dynamic_t dyn_view_road_hunter = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	 },
	view_scr_road_hunter
};

view_screen_t scr_road_hunter = {
	&dyn_view_road_hunter,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void view_scr_road_hunter() {
	view_render.setTextColor(WHITE);

	if (rh_game_state == RH_COMBAT_STATE_READY) {
	}
	else if (rh_game_state == RH_COMBAT_STATE_GAME_OVER) {
		rh_game_over_intro_display();
	}
	else {
		rh_game_frame_display();
		view_render.setTextSize(1);
		view_render.setCursor(2, 2);
		view_render.print("Score:");
		view_render.print((int)rh_score);
		if (rh_active_power != RH_POWER_NONE) {
			
			const char *pname = rh_game_get_power_name(rh_active_power);
			int secs		   = (int)rh_power_timer / 10;
			int len			   = rh_text_len(pname) + 1 + (secs >= 10 ? 2 : 1) + 1;
			view_render.setCursor(ROAD_SCREEN_WIDTH - (len * 6) - 2, 2);
			view_render.print(pname);
			view_render.print(" ");
			view_render.print(secs);
			view_render.print("s");
		}
	}
}

/*****************************************************************************/
/* Handle - Road Hunter game screen */
/*****************************************************************************/
void scr_road_hunter_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("scr_road_hunter_handle SCREEN_ENTRY\n");
		screen_clear();
		rh_game_reset_round();
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, AC_DISPLAY_SCREEN_IDLE_INTERVAL, TIMER_ONE_SHOT);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_RH_GAME_TICK, AC_DISPLAY_RH_GAME_TICK_INTERVAL, TIMER_PERIODIC);
	} break;

	case SCREEN_RESTORE: {
		APP_DBG_SIG("scr_road_hunter_handle SCREEN_RESTORE\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, AC_DISPLAY_SCREEN_IDLE_INTERVAL, TIMER_ONE_SHOT);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_RH_GAME_TICK, AC_DISPLAY_RH_GAME_TICK_INTERVAL, TIMER_PERIODIC);
	} break;

	case AC_DISPLAY_RH_GAME_TICK: {
		if (rh_game_state == RH_COMBAT_STATE_READY) {
			if (rh_state_timer > 0) {
				rh_state_timer--;
			}
			else {
				rh_game_state = RH_COMBAT_STATE_PLAYING;
			}
		}
		else if (rh_game_state == RH_COMBAT_STATE_GAME_OVER) {
			if (rh_state_timer > 0) {
				rh_state_timer--;
			}
			else {
				timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_RH_GAME_TICK);
				SCREEN_TRAN(scr_rh_game_over_handle, &scr_rh_game_over);
			}
		}
		else if (rh_game_state == RH_COMBAT_STATE_PLAYING) {
			rh_game_update();
		}
	} break;

	case AC_DISPLAY_BUTON_UP_PRESSED: {
		if (rh_game_state == RH_COMBAT_STATE_PLAYING) {
			if (rh_player_lane > 0) {
				rh_game_player_move_up();
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
			}
		}
	} break;

	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		if (rh_game_state == RH_COMBAT_STATE_PLAYING) {
			if (rh_player_lane < ROAD_LANE_COUNT - 1) {
				rh_game_player_move_down();
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
			}
		}
	} break;

	case AC_DISPLAY_SHOW_IDLE: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_IDLE\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_RH_GAME_TICK);
		SCREEN_TRAN(scr_idle_handle, &scr_idle);
	} break;

	default:
		break;
	}
}
