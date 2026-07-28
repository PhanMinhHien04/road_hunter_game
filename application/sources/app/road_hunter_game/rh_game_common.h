#ifndef __RH_GAME_COMMON_H__
#define __RH_GAME_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#define ROAD_LANE_COUNT		  5
#define ROAD_PLAYER_X		  16
#define ROAD_PLAYER_W		  14
#define ROAD_PLAYER_H		  9
#define ROAD_LANE_Y_BASE	  12
#define ROAD_LANE_STEP		  9
#define ROAD_SCREEN_WIDTH	128
/* Fixed playfield frame drawn every round (see rh_game_frame_display in
   scr_road_hunter.cpp): a horizontal border line above lane 0 and another
   below the last lane. Shared here so the menu's lane-5 preview
   (scr_rh_menu.cpp) can reuse the exact same coordinates instead of
   guessing/duplicating the literals. */
#define ROAD_TOP_BORDER_Y	  10
#define ROAD_BOTTOM_BORDER_Y  54
#define ROAD_BULLET_SPEED	  4
#define ROAD_ENEMY_SPEED_BASE 2
#define ROAD_MAX_ENEMIES	  6
#define ROAD_MAX_OBSTACLES	  8
#define ROAD_MAX_PLAYER_BULLETS 16
#define ROAD_MAX_ENEMY_BULLETS  	8
#define ROAD_MAX_BULLETS (ROAD_MAX_PLAYER_BULLETS + ROAD_MAX_ENEMY_BULLETS)
#define ROAD_MAX_CHESTS		  2

/* Game Over intro animation ("GAME"/"OVER" dropping in from the top of the
   screen, at AC_DISPLAY_RH_GAME_TICK's 120ms rate) played inside
   scr_road_hunter before the round auto-transitions to the framed
   scr_rh_game_over summary screen (Score/Record/Retry/Menu). */
#define RH_GAME_OVER_ANIM_FALL_TICKS  14 /* ticks for the text to drop into place */
#define RH_GAME_OVER_ANIM_HOLD_TICKS  10 /* ticks it stays settled before the transition */
#define RH_GAME_OVER_ANIM_TOTAL_TICKS (RH_GAME_OVER_ANIM_FALL_TICKS + RH_GAME_OVER_ANIM_HOLD_TICKS)

/* "Cuong hoa" (Empower): fills from destroyed tanks only, auto-activates when
   full. While active the player is invincible, rams straight through every
   enemy/obstacle/bullet it touches, moves faster, and cannot pick up chest
   powers. */
#define RH_EMPOWER_TANKS_TO_FULL	5
#define RH_EMPOWER_DURATION		83 /* ticks; ~10s at the 120ms game tick */
#define RH_EMPOWER_SPEED_BOOST	4

/* enemy widths */
#define RH_ENEMY_W_NORMAL 8

/* tank enemy: big vehicle with a gun barrel that fires bursts of rockets */
#define RH_TANK_W				  20
#define RH_TANK_HP				  2
#define RH_TANK_BURST_COUNT	  3	 /* rockets fired per burst */
#define RH_TANK_BURST_INTERVAL	  10 /* ticks between rockets within a burst */
#define RH_TANK_COOLDOWN		  45 /* ticks to wait before starting next burst */

/* obstacle widths for the "hard" barrier types (kept in one place so the
   collision code and the render code never drift apart) */
#define RH_OBSTACLE_W_FENCE	  10
#define RH_OBSTACLE_W_TRUCK	  26

/* score awarded per destroyed target */
#define RH_SCORE_ENEMY_NORMAL	  1
#define RH_SCORE_ENEMY_TANK	  3
#define RH_SCORE_VEHICLE_SMALL	  1
#define RH_SCORE_VEHICLE_BIG	  2

/* ------------------------------------------------------------------------ */
/* Boss battles: land tank / sea warship / sky aircraft, cycling in order   */
/* and appearing on a timer. While a boss fight is running the normal road  */
/* (enemies/obstacles/chests) is cleared -- it's just the boss and the      */
/* player until the boss is destroyed.                                     */
/* ------------------------------------------------------------------------ */
#define RH_BOSS_LANE_LO			  1	 /* default first lane of a 3-lane boss band */
#define RH_BOSS_LANE_COUNT		  3	 /* width, in lanes, of the land/sky boss band */
#define RH_BOSS_SCORE				  50
#define RH_BOSS_FIRST_DELAY_TICKS	  167 /* 20s at 120ms/tick before the first boss */
#define RH_BOSS_NEXT_DELAY_TICKS	  167 /* 20s at 120ms/tick between boss fights */
#define RH_BOSS_INTRO_TICKS		  14
#define RH_BOSS_OUTRO_TICKS		  12


/* the sea boss is a full-width warship: it occupies every lane on the road */
#define RH_BOSS_SEA_LANE_LO		  0
#define RH_BOSS_SEA_LANE_COUNT		  ROAD_LANE_COUNT

/* land boss: ticks between each 1-lane step of its up/down patrol across
   the whole 5-lane road (band top ranges 0..ROAD_LANE_COUNT-RH_BOSS_LANE_COUNT) */
#define RH_BOSS_LAND_MOVE_TICKS	  12

#define RH_BOSS_LAND_HP			  30
#define RH_BOSS_SEA_HP				  30
#define RH_BOSS_SKY_HP				  30

#define RH_BOSS_LAND_X				  92
#define RH_BOSS_LAND_W				  32
#define RH_BOSS_SEA_X				  88
#define RH_BOSS_SEA_W				  36
#define RH_BOSS_SKY_X				  90
#define RH_BOSS_SKY_W				  32
#define RH_BOSS_SLIDE_IN_X			  140

/* the sky boss's wing weak point only lives at the wing's own center
   point (matches the middle of the wing triangle drawn from x0+2 to
   x0+w-8 in the view) -- a shot has to land within this many pixels of
   that center to count, not just anywhere along the wing's span */
#define RH_BOSS_SKY_WING_CENTER_OFFSET  13
#define RH_BOSS_SKY_WING_TOL			  3

#define ROAD_MAX_BOSS_PROJECTILES	  20

enum rh_boss_type {
	RH_BOSS_NONE = 0,
	RH_BOSS_LAND = 1, /* tank, 10 hp */
	RH_BOSS_SEA	 = 2, /* warship/submarine, 20 hp */
	RH_BOSS_SKY	 = 3  /* aircraft, 20 hp */
};

enum rh_boss_phase {
	RH_BOSS_PHASE_NONE	= 0, /* no boss encounter running */
	RH_BOSS_PHASE_INTRO = 1, /* map/vehicle transition, boss sliding in */
	RH_BOSS_PHASE_FIGHT = 2, /* boss is live and attacking */
	RH_BOSS_PHASE_OUTRO = 3	 /* boss destroyed, playing out the explosion */
};

enum rh_boss_proj_type {
	RH_BPROJ_LAND_SHELL = 0, /* land: big round shell, can be shot down */
	RH_BPROJ_MINE		= 1, /* land: scattered mine, cannot be shot, must dodge */
	RH_BPROJ_TORPEDO	= 2, /* sea: torpedo, cannot be shot, must dodge */
	RH_BPROJ_MISSILE	= 3	 /* sky: missile, cannot be shot, must dodge */
};

typedef struct {
	bool active;
	uint8_t type; /* rh_boss_proj_type */
	int16_t x;
	uint8_t lane;
	int8_t vx;
} rh_boss_projectile_t;

typedef struct {
	uint8_t type;  /* rh_boss_type */
	uint8_t phase; /* rh_boss_phase */
	int16_t x;	   /* left edge of the boss body */
	int16_t hp;
	int16_t max_hp;
	uint16_t phase_timer; /* used by INTRO/OUTRO */

	/* the one lane that currently takes damage, blinking to a new lane
	   every so often -- same mechanic for land (0..2, relative to
	   land_base_lane) and sea (0..ROAD_LANE_COUNT-1, absolute lane,
	   since the ship already spans the whole road) */
	uint8_t weak_slot;
	uint16_t weak_timer;

	/* generic attack cooldown, reused by all 3 boss types for their main
	   projectile skill (shell / torpedo volley / missile) */
	uint16_t fire_timer;
	/* land only: separate cooldown for scattering mines */
	uint16_t mine_timer;

	/* land: top lane (0..ROAD_LANE_COUNT-RH_BOSS_LANE_COUNT) of the moving
	   3-lane band the tank currently occupies -- the tank continuously
	   patrols up and down the full road, band_lo shifting by 1 lane every
	   RH_BOSS_LAND_MOVE_TICKS ticks and bouncing at each end; its 3 gun
	   barrels track this band so shells only ever leave from a real barrel */
	uint8_t land_base_lane;
	int8_t land_move_dir;
	uint16_t land_move_timer;

	/* sky: base lane of the moving 3-lane band (body occupies
	   [sky_base_lane, sky_base_lane+2]); wings are the top/bottom lanes of
	   that band and are only damageable while wing_lit is true */
	uint8_t sky_base_lane;
	uint16_t sky_move_timer;
	bool wing_lit;
	uint16_t wing_blink_timer;
} rh_game_boss_t;

enum rh_combat_screen_state {
	RH_COMBAT_STATE_READY	   = 0,
	RH_COMBAT_STATE_PLAYING   = 1,
	RH_COMBAT_STATE_GAME_OVER = 3
};

enum rh_combat_power_up {
	RH_POWER_NONE		= 0,
	RH_POWER_GATLING	= 1,
	RH_POWER_PIERCING	= 4,
	RH_POWER_EXPLOSIVE = 5,
	RH_POWER_SPIDER	= 6,
	RH_POWER_LASER		= 7,
	RH_POWER_SHIELD		= 9
};

typedef struct {
	int16_t x;
	uint8_t lane;
	uint8_t hp;
	uint8_t fire_timer;
	uint8_t rockets_fired;
	bool active;
	bool trapped;
	uint8_t trap_timer;
	bool is_tank;
} rh_game_enemy_t;

typedef struct {
	int16_t x;
	uint8_t lane;
	bool active;
	uint8_t type;
	int8_t hp;
} rh_game_obstacle_t;

typedef struct {
	int16_t x;
	uint8_t lane;
	bool active;
	uint8_t damage;
	bool piercing;
	uint8_t type;
	bool is_enemy;
	int8_t vx;
	int8_t vy;
} rh_game_bullet_t;

typedef struct {
	int16_t x;
	uint8_t lane;
	bool active;
} rh_game_chest_t;

static inline int rh_lane_to_y(uint8_t lane) {
	return ROAD_LANE_Y_BASE + lane * ROAD_LANE_STEP;
}

static inline int rh_enemy_width(const rh_game_enemy_t *e) {
	return e->is_tank ? RH_TANK_W : RH_ENEMY_W_NORMAL;
}

static inline int rh_enemy_score_value(const rh_game_enemy_t *e) {
	return e->is_tank ? RH_SCORE_ENEMY_TANK : RH_SCORE_ENEMY_NORMAL;
}

static inline int rh_obstacle_score_value(const rh_game_obstacle_t *o) {
	/* type 5 = big car, type 4 = small car, everything else defaults to 1 */
	return (o->type == 5) ? RH_SCORE_VEHICLE_BIG : RH_SCORE_VEHICLE_SMALL;
}

#endif //__RH_GAME_COMMON_H__