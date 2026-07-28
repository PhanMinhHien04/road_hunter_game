#include "scr_rh_title.h"

/*****************************************************************************/
/* View - Road Hunter TITLE screen                                          */
/* Shown after the AK boot logo (scr_startup) and before the main menu      */
/* (scr_rh_menu). Displays the game name "ROAD HUNTER" and, underneath it,  */
/* the exact 3 forms the player's vehicle takes in each of the 3 boss zones */
/* -- same drawing code as the in-game boss fight (scr_road_hunter.cpp):    */
/*   - Land zone : normal car + cannon, driving on a plain dashed road      */
/*   - Sea zone  : boat hull + mast/flag + cannon, driving on wavy water    */
/*   - Sky zone  : winged car + cannon, driving through drifting clouds     */
/*****************************************************************************/

#define RH_TITLE_VEH_W		(CAR_SPRITE_W) /* plays the role of ROAD_PLAYER_W */
#define RH_TITLE_VEH_H		(CAR_SPRITE_H) /* plays the role of ROAD_PLAYER_H */

/* left x of the vehicle in each row (distance from the left edge):
   - land : hugs the left edge
   - sea  : centered on the screen
   - sky  : sits exactly halfway between the land car and the sea boat
   so the 3 cars start from clearly different points instead of all
   lining up in the same column. */
#define RH_TITLE_ROW0_X		(10)								   /* land row left x -- sat trai */
#define RH_TITLE_ROW1_X		((128 - RH_TITLE_VEH_W) / 2)		   /* sea row  left x -- giua man hinh */
#define RH_TITLE_ROW2_X		((RH_TITLE_ROW0_X + RH_TITLE_ROW1_X) / 2) /* sky row left x -- giua xe1 va xe2 */

#define RH_TITLE_ROW0_Y		(12)  /* land row (py baseline) */
#define RH_TITLE_ROW1_Y		(30)  /* sea row  (py baseline) */
#define RH_TITLE_ROW2_Y		(48)  /* sky row  (py baseline) */

static uint16_t rh_title_tick = 0;

static void view_scr_rh_title();

view_dynamic_t dyn_view_rh_title = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_rh_title
};

view_screen_t scr_rh_title = {
	&dyn_view_rh_title,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

/* Rocket booster flame + drifting smoke trail behind the car -- identical to
   rh_game_player_thruster_display() in scr_road_hunter.cpp, just driven by
   this screen's own tick counter instead of rh_game_tick, and taking the
   row's own left x (px) instead of a single shared position. */
static void rh_title_thruster_display(int px, int py) {
	int by = py + (RH_TITLE_VEH_H / 2) - 1;
	int bx = px - 1;
	int flicker = rh_title_tick & 0x03;
	int flame_len = 3 + (flicker < 2 ? 1 : 0);

	/* bright flame core, right behind the exhaust */
	view_render.fillTriangle(bx, by - 1, bx, by + 2, bx - flame_len, by, WHITE);

	/* smoke puffs: dense near the car, thinning out with distance */
	int p0x = bx - flame_len - 2 - (rh_title_tick % 3);
	view_render.fillRect(p0x, by - 1, 2, 2, WHITE);

	int p1x = bx - flame_len - 6 - (rh_title_tick % 3);
	int p1y = by + ((rh_title_tick / 2) % 3) - 1;
	view_render.drawPixel(p1x, p1y, WHITE);
	view_render.drawPixel(p1x + 1, p1y + 1, WHITE);

	int p2x = bx - flame_len - 10 - (rh_title_tick % 4);
	int p2y = by + ((rh_title_tick / 3) % 3) - 1;
	if ((rh_title_tick & 1) == 0) {
		view_render.drawPixel(p2x, p2y, WHITE);
	}
}

/* Puddle-style water ripple -- identical to rh_draw_ripple() in scr_road_hunter.cpp */
static void rh_title_ripple(int cx, int cy, int phase) {
	int step = (rh_title_tick + phase) % 12;
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

/* Combat boat hull -- identical to rh_game_player_boat_hull_display(), now
   anchored on the row's own left x (px). */
static void rh_title_boat_hull_display(int px, int py) {
	int bx = px;
	int by = py + 1;				  /* deck line */
	int bb = py + RH_TITLE_VEH_H;	  /* hull bottom / waterline */

	view_render.fillRect(bx, by, RH_TITLE_VEH_W - 4, bb - by, WHITE);
	view_render.fillTriangle(bx + RH_TITLE_VEH_W - 4, by, bx + RH_TITLE_VEH_W + 2, (by + bb) / 2,
		bx + RH_TITLE_VEH_W - 4, bb, WHITE);
	view_render.fillRect(bx - 1, by + 1, 2, bb - by - 2, WHITE); /* transom stern */

	/* small pilot cabin amidships with a window slit */
	view_render.fillRect(bx + 3, by - 4, 6, 4, WHITE);
	view_render.drawFastHLine(bx + 4, by - 3, 4, BLACK);

	/* waterline ripples lapping at the hull */
	rh_title_ripple(bx - 1, bb + 2, 0);
	rh_title_ripple(bx + 9, bb + 2, 6);
}

/* Heavy cannon mounted on the bow, muzzle brake at the tip -- identical to the
   cannon block inside rh_game_player_display(). Drawn in every row: the boss
   battle keeps this cannon regardless of land/sea/sky form. Now anchored on
   the row's own left x (px). */
static void rh_title_cannon_display(int px, int py) {
	int gx = px + RH_TITLE_VEH_W;
	int gy = py + (RH_TITLE_VEH_H / 2) - 2;
	view_render.drawFastHLine(gx, gy, 5, WHITE);
	view_render.drawFastHLine(gx, gy + 1, 5, WHITE);
	view_render.fillRect(gx + 4, gy - 1, 2, 4, WHITE);
}

/* Row 1: land zone -- plain dashed road, normal player car + cannon, hugging
   the left edge (RH_TITLE_ROW0_X). Matches the land-boss state of
   rh_game_player_display(): is_boat==false and rh_game_player_boss_form_display()
   draws nothing extra for RH_BOSS_LAND. */
static void rh_title_row_land(int py) {
	const int px = RH_TITLE_ROW0_X;
	int line_y = py + RH_TITLE_VEH_H - 1;
	for (int x = -(rh_title_tick % 20); x < 128; x += 20) {
		view_render.drawFastHLine(x, line_y, 10, WHITE);
	}

	rh_title_thruster_display(px, py);
	view_render.drawBitmap(px, py - 2, bitmap_car, CAR_SPRITE_W, CAR_SPRITE_H, WHITE);
	rh_title_cannon_display(px, py);
}

/* Row 2: sea zone -- wavy water, player's boat-hull form + mast/flag +
   cannon, centered on the screen (RH_TITLE_ROW1_X). Matches the sea-boss
   state: is_boat==true (rh_game_player_boat_hull_display) plus the
   RH_BOSS_SEA branch of rh_game_player_boss_form_display() (mast+flag). */
static void rh_title_row_sea(int py) {
	const int px = RH_TITLE_ROW1_X;
	for (int lane = 0; lane < 2; lane++) {
		int y = py - 9 + lane * 18;
		for (int x = -(rh_title_tick % 8); x < 128; x += 8) {
			int wy = y + (((x / 4) + rh_title_tick) % 2);
			view_render.drawFastHLine(x, wy, 3, WHITE);
		}
	}

	rh_title_thruster_display(px, py);
	rh_title_boat_hull_display(px, py);
	rh_title_cannon_display(px, py);

	/* mast + fluttering flag (RH_BOSS_SEA branch of rh_game_player_boss_form_display) */
	int deck_x = px + (RH_TITLE_VEH_W / 2) - 3;
	int deck_y = py - 3; /* top of the pilot cabin */
	view_render.drawFastVLine(deck_x + 1, deck_y - 4, 4, WHITE);
	int flap = (rh_title_tick / 4) % 2;
	view_render.drawLine(deck_x + 2, deck_y - 4, deck_x + 4 + flap, deck_y - 3, WHITE);
}

/* Row 3: sky zone -- drifting clouds, player car + cannon + the pair of
   wings it sprouts during the sky boss fight, sitting halfway between the
   land car and the sea boat (RH_TITLE_ROW2_X). Matches the sky-boss state:
   is_boat==false plus the RH_BOSS_SKY branch of rh_game_player_boss_form_display(). */
static void rh_title_row_sky(int py) {
	const int px = RH_TITLE_ROW2_X;
	for (int x = -(rh_title_tick % 44); x < 128; x += 44) {
		view_render.drawFastHLine(x, py - 10, 6, WHITE);
		view_render.drawPixel(x + 8, py - 10, WHITE);
	}
	for (int x = -((rh_title_tick * 2) % 56); x < 128; x += 56) {
		view_render.drawFastHLine(x, py + 12, 5, WHITE);
	}

	rh_title_thruster_display(px, py);
	view_render.drawBitmap(px, py - 2, bitmap_car, CAR_SPRITE_W, CAR_SPRITE_H, WHITE);
	rh_title_cannon_display(px, py);

	/* wings (RH_BOSS_SKY branch of rh_game_player_boss_form_display) */
	int cy = py + (RH_TITLE_VEH_H / 2);
	view_render.fillTriangle(px + 3, cy, px - 5, cy - 6, px + 7, cy - 2, WHITE);
	view_render.fillTriangle(px + 3, cy, px - 5, cy + 6, px + 7, cy + 2, WHITE);
}

static void view_scr_rh_title() {
	rh_title_row_land(RH_TITLE_ROW0_Y);
	rh_title_row_sea(RH_TITLE_ROW1_Y);
	rh_title_row_sky(RH_TITLE_ROW2_Y);
}

void scr_rh_title_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		screen_clear();
		APP_DBG_SIG("SCREEN_ENTRY\n");
		rh_title_tick = 0;
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_TITLE_ANIM_TICK, AC_DISPLAY_TITLE_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_TITLE_SHOW_MENU, AC_DISPLAY_TITLE_INTERVAL, TIMER_ONE_SHOT);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, AC_DISPLAY_SCREEN_IDLE_INTERVAL, TIMER_ONE_SHOT);
	} break;

	case AC_DISPLAY_TITLE_ANIM_TICK: {
		rh_title_tick++;
	} break;

	case AC_DISPLAY_TITLE_SHOW_MENU: {
		APP_DBG_SIG("AC_DISPLAY_TITLE_SHOW_MENU\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_TITLE_ANIM_TICK);
		SCREEN_TRAN(scr_rh_menu_handle, &scr_rh_menu);
	} break;

	case AC_DISPLAY_BUTON_MODE_PRESSED:
	case AC_DISPLAY_BUTON_UP_PRESSED:
	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTON_%s_PRESSED\n", msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED ? "MODE" :\
													 msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED ? "DOWN" : "UP");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_TITLE_ANIM_TICK);
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_TITLE_SHOW_MENU);
		SCREEN_TRAN(scr_rh_menu_handle, &scr_rh_menu);
	} break;

	default:
		break;
	}
}