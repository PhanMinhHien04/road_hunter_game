#include "rh_game_boss.h"

#include <stdlib.h>

#include "rh_game_bullet.h"
#include "rh_game_chest.h"
#include "rh_game_core.h"
#include "rh_game_player.h"

#include "buzzer.h"

rh_game_boss_t rh_boss;
rh_boss_projectile_t rh_boss_projectiles[ROAD_MAX_BOSS_PROJECTILES];
uint16_t rh_boss_countdown = RH_BOSS_FIRST_DELAY_TICKS;

/* 0 = land, 1 = sea, 2 = sky -- cycles forever in this order */
static uint8_t rh_boss_cycle = 0;

static void rh_boss_projectiles_clear(void) {
	for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
		rh_boss_projectiles[i].active = false;
	}
}

void rh_game_boss_reset(void) {
	rh_boss.type		   = RH_BOSS_NONE;
	rh_boss.phase		   = RH_BOSS_PHASE_NONE;
	rh_boss.x			   = RH_BOSS_SLIDE_IN_X;
	rh_boss.hp			   = 0;
	rh_boss.max_hp		   = 0;
	rh_boss.phase_timer   = 0;
	rh_boss.weak_slot	   = 0;
	rh_boss.weak_timer	   = 0;
	rh_boss.fire_timer	   = 0;
	rh_boss.mine_timer	   = 0;
	rh_boss.sky_base_lane  = 1;
	rh_boss.sky_move_timer = 0;
	rh_boss.wing_lit	   = true;
	rh_boss.wing_blink_timer = 0;

	rh_boss.land_base_lane	 = RH_BOSS_LANE_LO;
	rh_boss.land_move_dir	 = 1;
	rh_boss.land_move_timer = RH_BOSS_LAND_MOVE_TICKS;

	rh_boss_projectiles_clear();

	rh_boss_countdown = RH_BOSS_FIRST_DELAY_TICKS;
	rh_boss_cycle	   = 0;
}

bool rh_game_boss_active(void) {
	return rh_boss.phase != RH_BOSS_PHASE_NONE;
}

static int16_t rh_boss_rest_x(uint8_t type) {
	switch (type) {
	case RH_BOSS_LAND: return RH_BOSS_LAND_X;
	case RH_BOSS_SEA:  return RH_BOSS_SEA_X;
	case RH_BOSS_SKY:  return RH_BOSS_SKY_X;
	default:		   return RH_BOSS_LAND_X;
	}
}

static int16_t rh_boss_width(uint8_t type) {
	switch (type) {
	case RH_BOSS_LAND: return RH_BOSS_LAND_W;
	case RH_BOSS_SEA:  return RH_BOSS_SEA_W;
	case RH_BOSS_SKY:  return RH_BOSS_SKY_W;
	default:		   return RH_BOSS_LAND_W;
	}
}

void rh_game_boss_start_next(void) {
	static const uint8_t order[3] = { RH_BOSS_LAND, RH_BOSS_SEA, RH_BOSS_SKY };
	uint8_t type = order[rh_boss_cycle % 3];
	rh_boss_cycle++;

	/* boss fights are boss + player only: no chests/cars/rocks/fences */
	rh_game_clear_stage_entities();
	rh_boss_projectiles_clear();

	/* reset player to normal bullets when entering any boss fight --
	   clear active power, timer, shield, and the shoot-rate timer so
	   the player fires at the default cadence from the very first tick
	   of the encounter, regardless of which power was active before. */
	rh_active_power  = RH_POWER_NONE;
	rh_power_timer   = 0;
	rh_shield_active = false;
	rh_game_bullet_reset_shoot_timer();

	rh_boss.type		 = type;
	rh_boss.phase		 = RH_BOSS_PHASE_INTRO;
	rh_boss.phase_timer = RH_BOSS_INTRO_TICKS;
	rh_boss.x			 = RH_BOSS_SLIDE_IN_X;

	rh_boss.weak_slot  = (type == RH_BOSS_SEA) ? (rand() % ROAD_LANE_COUNT) : (rand() % RH_BOSS_LANE_COUNT);
	rh_boss.weak_timer = 30;
	rh_boss.fire_timer = 40;
	rh_boss.mine_timer = 18; /* mines start scattering quickly */

	rh_boss.sky_base_lane	 = 1;
	rh_boss.sky_move_timer	 = 40;
	rh_boss.wing_lit		 = true;
	rh_boss.wing_blink_timer = 14;

	rh_boss.land_base_lane	 = RH_BOSS_LANE_LO;
	rh_boss.land_move_dir	 = 1;
	rh_boss.land_move_timer = RH_BOSS_LAND_MOVE_TICKS;

	switch (type) {
	case RH_BOSS_LAND: rh_boss.hp = RH_BOSS_LAND_HP; rh_boss.max_hp = RH_BOSS_LAND_HP; break;
	case RH_BOSS_SEA:  rh_boss.hp = RH_BOSS_SEA_HP;  rh_boss.max_hp = RH_BOSS_SEA_HP;  break;
	case RH_BOSS_SKY:  rh_boss.hp = RH_BOSS_SKY_HP;  rh_boss.max_hp = RH_BOSS_SKY_HP;  break;
	default: break;
	}

	if (rh_sound_enabled) {
		BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
	}
}

/* middle lane of whichever band the boss currently occupies (land and sky
   bands move, so this can't be a fixed constant anymore) */
static uint8_t rh_boss_mid_lane(void) {
	switch (rh_boss.type) {
	case RH_BOSS_LAND: return rh_boss.land_base_lane + 1;
	case RH_BOSS_SEA:  return ROAD_LANE_COUNT / 2;
	case RH_BOSS_SKY:  return rh_boss.sky_base_lane + 1;
	default:		   return RH_BOSS_LANE_LO + 1;
	}
}

/* random lane within whichever band the boss currently occupies, used for
   scattering the outro death explosions across its whole body */
static uint8_t rh_boss_random_lane(void) {
	switch (rh_boss.type) {
	case RH_BOSS_LAND: return rh_boss.land_base_lane + (rand() % RH_BOSS_LANE_COUNT);
	case RH_BOSS_SEA:  return rand() % ROAD_LANE_COUNT;
	case RH_BOSS_SKY:  return rh_boss.sky_base_lane + (rand() % RH_BOSS_LANE_COUNT);
	default:		   return RH_BOSS_LANE_LO + (rand() % RH_BOSS_LANE_COUNT);
	}
}

static void rh_boss_take_damage(int dmg) {
	rh_boss.hp -= dmg;
	if (rh_boss.hp <= 0) {
		rh_boss.hp			 = 0;
		rh_score			+= RH_BOSS_SCORE;
		rh_boss.phase		 = RH_BOSS_PHASE_OUTRO;
		rh_boss.phase_timer = RH_BOSS_OUTRO_TICKS;
		rh_boss_projectiles_clear();
		rh_game_explosion_spawn(rh_boss.x + (rh_boss_width(rh_boss.type) / 2),
			rh_boss_mid_lane());
		if (rh_sound_enabled) {
			BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
		}
	}
}

/* Move every live boss projectile and check it against the player. Shared by
   all 3 boss types since shells/mines/torpedoes/missiles all just travel
   left toward the player at their own speed. Returns true on game over. */
static bool rh_boss_projectiles_update(void) {
	for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
		if (!rh_boss_projectiles[i].active)
			continue;
		rh_boss_projectile_t *p = &rh_boss_projectiles[i];
		p->x += p->vx;
		if (p->x < -8) {
			p->active = false;
			continue;
		}
		if (p->lane == rh_player_lane &&
			p->x <= ROAD_PLAYER_X + ROAD_PLAYER_W &&
			p->x + 4 >= ROAD_PLAYER_X) {
			if (rh_empower_active) {
				p->active = false;
				continue;
			}
			if (rh_game_consume_shield()) {
				p->active = false;
				if (rh_sound_enabled) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
				continue;
			}
			rh_game_trigger_over();
			return true;
		}
	}
	return false;
}

/* ---- LAND: tank boss ---- */
static bool rh_game_boss_update_land(void) {
	/* continuous up/down patrol: the tank's 3-lane band steadily walks the
	   whole 5-lane road and bounces back at each end, instead of sitting
	   still -- its 3 gun barrels move with it */
	if (rh_boss.land_move_timer > 0) {
		rh_boss.land_move_timer--;
	}
	else {
		rh_boss.land_move_timer = RH_BOSS_LAND_MOVE_TICKS;
		int8_t max_base			 = ROAD_LANE_COUNT - RH_BOSS_LANE_COUNT;
		int16_t next			 = (int16_t)rh_boss.land_base_lane + rh_boss.land_move_dir;
		if (next < 0 || next > max_base) {
			rh_boss.land_move_dir = (int8_t)(-rh_boss.land_move_dir);
			next				  = (int16_t)rh_boss.land_base_lane + rh_boss.land_move_dir;
		}
		rh_boss.land_base_lane = (uint8_t)next;
	}

	/* weak point randomly blinks to a different one of the 3 lanes */
	if (rh_boss.weak_timer > 0) {
		rh_boss.weak_timer--;
	}
	else {
		rh_boss.weak_timer = 20 + (rand() % 10);
		uint8_t next_slot;
		do {
			next_slot = rand() % RH_BOSS_LANE_COUNT;
		} while (next_slot == rh_boss.weak_slot);
		rh_boss.weak_slot = next_slot;
	}

	/* skill: big round shell fired from one of the 3 barrels, shootable
	   by the player. It can only leave from a real barrel, so its lane is
	   always inside the tank's current band. */
	if (rh_boss.fire_timer > 0) {
		rh_boss.fire_timer--;
	}
	else {
		rh_boss.fire_timer = 42;
		for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
			if (!rh_boss_projectiles[i].active) {
				rh_boss_projectiles[i].active = true;
				rh_boss_projectiles[i].type	= RH_BPROJ_LAND_SHELL;
				rh_boss_projectiles[i].x		= rh_boss.x - 12; /* barrel muzzle tip */
				rh_boss_projectiles[i].lane	= rh_boss.land_base_lane + (rand() % RH_BOSS_LANE_COUNT);
				rh_boss_projectiles[i].vx		= -5;
				break;
			}
		}
	}

	/* skill: minefield scattered across any lane of the road, cannot be
	   shot, only dodged. Spawns noticeably more often than before. */
	if (rh_boss.mine_timer > 0) {
		rh_boss.mine_timer--;
	}
	else {
		rh_boss.mine_timer = 20 + (rand() % 14);
		for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
			if (!rh_boss_projectiles[i].active) {
				rh_boss_projectiles[i].active = true;
				rh_boss_projectiles[i].type	= RH_BPROJ_MINE;
				rh_boss_projectiles[i].x		= rh_boss.x - 10 - (rand() % 60);
				rh_boss_projectiles[i].lane	= rand() % ROAD_LANE_COUNT;
				rh_boss_projectiles[i].vx		= -2;
				break;
			}
		}
	}

	return rh_boss_projectiles_update();
}

/* ---- SEA: warship boss ---- */
static bool rh_game_boss_update_sea(void) {
	/* weak point mechanic identical to the land boss: one lane out of the
	   ship's full width blinks open at a time and is the only spot that
	   takes damage; everywhere else absorbs the hit harmlessly */
	if (rh_boss.weak_timer > 0) {
		rh_boss.weak_timer--;
	}
	else {
		rh_boss.weak_timer = 20 + (rand() % 10);
		uint8_t next_slot;
		do {
			next_slot = rand() % ROAD_LANE_COUNT;
		} while (next_slot == rh_boss.weak_slot);
		rh_boss.weak_slot = next_slot;
	}

	if (rh_boss.fire_timer > 0) {
		rh_boss.fire_timer--;
	}
	else {
		rh_boss.fire_timer = 26; /* fires more often */
		/* pick 4 distinct lanes out of the 5 for this bigger torpedo volley */
		uint8_t lanes[ROAD_LANE_COUNT] = { 0, 1, 2, 3, 4 };
		for (int i = ROAD_LANE_COUNT - 1; i > 0; i--) {
			int j = rand() % (i + 1);
			uint8_t t = lanes[i];
			lanes[i]  = lanes[j];
			lanes[j]  = t;
		}
		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
				if (!rh_boss_projectiles[i].active) {
					rh_boss_projectiles[i].active = true;
					rh_boss_projectiles[i].type	= RH_BPROJ_TORPEDO;
					rh_boss_projectiles[i].x		= rh_boss.x - 12; /* torpedo tube mouth */
					rh_boss_projectiles[i].lane	= lanes[k];
					rh_boss_projectiles[i].vx		= -4;
					break;
				}
			}
		}
	}

	return rh_boss_projectiles_update();
}

/* ---- SKY: aircraft boss ---- */
static bool rh_game_boss_update_sky(void) {
	/* boss keeps shifting lane-band to dodge player fire */
	if (rh_boss.sky_move_timer > 0) {
		rh_boss.sky_move_timer--;
	}
	else {
		rh_boss.sky_move_timer = 40 + (rand() % 20);
		uint8_t next_base;
		do {
			next_base = rand() % (ROAD_LANE_COUNT - 2);
		} while (next_base == rh_boss.sky_base_lane);
		rh_boss.sky_base_lane = next_base;
	}

	/* wings slowly blink between vulnerable (lit) and armored */
	if (rh_boss.wing_blink_timer > 0) {
		rh_boss.wing_blink_timer--;
	}
	else {
		rh_boss.wing_blink_timer = 14;
		rh_boss.wing_lit		  = !rh_boss.wing_lit;
	}

	/* skill: missiles fired at random lanes, cannot be shot, only dodged */
	if (rh_boss.fire_timer > 0) {
		rh_boss.fire_timer--;
	}
	else {
		rh_boss.fire_timer = 22;
		for (int i = 0; i < ROAD_MAX_BOSS_PROJECTILES; i++) {
			if (!rh_boss_projectiles[i].active) {
				rh_boss_projectiles[i].active = true;
				rh_boss_projectiles[i].type	= RH_BPROJ_MISSILE;
				rh_boss_projectiles[i].x		= ROAD_SCREEN_WIDTH;
				rh_boss_projectiles[i].lane	= rand() % ROAD_LANE_COUNT;
				rh_boss_projectiles[i].vx		= -4;
				break;
			}
		}
	}

	return rh_boss_projectiles_update();
}

bool rh_game_boss_update(int move_speed) {
	(void)move_speed; /* bosses are stationary in x; kept for a uniform signature */

	if (rh_boss.phase == RH_BOSS_PHASE_NONE)
		return false;

	if (rh_boss.phase == RH_BOSS_PHASE_INTRO) {
		int16_t rest_x = rh_boss_rest_x(rh_boss.type);
		if (rh_boss.x > rest_x) {
			rh_boss.x -= 4;
			if (rh_boss.x < rest_x) {
				rh_boss.x = rest_x;
			}
		}
		if (rh_boss.phase_timer > 0) {
			rh_boss.phase_timer--;
		}
		else {
			rh_boss.x	  = rest_x;
			rh_boss.phase = RH_BOSS_PHASE_FIGHT;
		}
		return false;
	}

	if (rh_boss.phase == RH_BOSS_PHASE_OUTRO) {
		if (rh_boss.phase_timer > 0) {
			rh_boss.phase_timer--;
			if ((rh_boss.phase_timer & 1) == 0) {
				rh_game_explosion_spawn(rh_boss.x + (rand() % rh_boss_width(rh_boss.type)), rh_boss_random_lane());
			}
		}
		else {
			/* boss fully defeated: back to the normal road, timer for next boss */
			rh_boss.type  = RH_BOSS_NONE;
			rh_boss.phase = RH_BOSS_PHASE_NONE;
			rh_boss_projectiles_clear();
			rh_boss_countdown = RH_BOSS_NEXT_DELAY_TICKS;
		}
		return false;
	}

	/* RH_BOSS_PHASE_FIGHT */
	switch (rh_boss.type) {
	case RH_BOSS_LAND: return rh_game_boss_update_land();
	case RH_BOSS_SEA:  return rh_game_boss_update_sea();
	case RH_BOSS_SKY:  return rh_game_boss_update_sky();
	default: return false;
	}
}

void rh_game_boss_check_bullet_hits(void) {
	if (rh_boss.phase != RH_BOSS_PHASE_FIGHT)
		return;

	int16_t x0 = rh_boss.x;
	int16_t w  = rh_boss_width(rh_boss.type);

	for (int i = 0; i < ROAD_MAX_PLAYER_BULLETS; i++) {
		if (!rh_player_bullets[i].active)
			continue;
		rh_game_bullet_t *b = &rh_player_bullets[i];

		/* shootable projectiles (land shells) take priority: bullet and
		   shell mutually destroy each other */
		bool consumed = false;
		for (int j = 0; j < ROAD_MAX_BOSS_PROJECTILES; j++) {
			if (!rh_boss_projectiles[j].active || rh_boss_projectiles[j].type != RH_BPROJ_LAND_SHELL)
				continue;
			if (rh_boss_projectiles[j].lane != b->lane)
				continue;
			if (b->x + 3 >= rh_boss_projectiles[j].x - 2 && b->x <= rh_boss_projectiles[j].x + 4) {
				rh_boss_projectiles[j].active = false;
				b->active					   = false;
				rh_game_explosion_spawn(rh_boss_projectiles[j].x, rh_boss_projectiles[j].lane);
				consumed = true;
				break;
			}
		}
		if (consumed || !b->active)
			continue;

		if (b->x + 3 < x0 || b->x > x0 + w)
			continue; /* bullet is not over the boss body x-range at all */

		switch (rh_boss.type) {
		case RH_BOSS_LAND: {
			int slot = b->lane - rh_boss.land_base_lane;
			if (slot < 0 || slot >= RH_BOSS_LANE_COUNT)
				break; /* passed above/below the tank's current band entirely */
			b->active = false;
			if (slot == rh_boss.weak_slot) {
				rh_boss_take_damage(1);
				rh_game_explosion_spawn(b->x, b->lane);
			}
			/* otherwise the shot is absorbed by the armor: no damage */
			break;
		}
		case RH_BOSS_SEA: {
			/* the warship spans every lane, but only the currently
			   blinking lane is a real weak point -- exactly like the
			   land boss's hatch mechanic */
			b->active = false;
			if (b->lane == rh_boss.weak_slot) {
				rh_boss_take_damage(1);
				rh_game_explosion_spawn(b->x, b->lane);
			}
			/* otherwise the shot is absorbed by the hull: no damage */
			break;
		}
		case RH_BOSS_SKY: {
			int top = rh_boss.sky_base_lane;
			int bot = rh_boss.sky_base_lane + (RH_BOSS_LANE_COUNT - 1);
			if (b->lane < top || b->lane > bot)
				break; /* missed the aircraft entirely */
			if (b->lane == top || b->lane == bot) {
				/* Wing lane: the wing triangle is drawn from x0+2 to x0+w-8
				   with an engine pod at x0+w-8 to x0+w-2.
				   Only consume the bullet once it has visually reached
				   the wing left edge (x0+2) -- no gap between bullet and wing. */
				if (b->x + 3 < x0 + 2)
					break; /* bullet hasn't reached the wing yet -- keep flying */
				b->active = false;
				if (rh_boss.wing_lit) {
					/* Any hit anywhere on a lit wing deals 1 HP damage */
					rh_boss_take_damage(1);
					rh_game_explosion_spawn(b->x, b->lane);
				}
				/* wing is armored (not lit): bullet absorbed, no damage */
			}
			else {
				/* Fuselage (middle lane): bullet is absorbed, no damage */
				b->active = false;
			}
			break;
		}
		default: break;
		}
	}
}