#include "screens.h"
#include "rh_game_core.h"
#include "scr_game_bitmap.h"


#define RH_MENU_ITEM_START    (0)
#define RH_MENU_ITEM_RECORD   (1)
#define RH_MENU_ITEM_SETTING  (2)
#define RH_MENU_ITEM_COUNT    (3)

/* 4 equal rows across the 64px screen height. */
#define RH_MENU_ROW_H			(16)
#define RH_MENU_ROW_TITLE_Y		(0)
#define RH_MENU_ROW_LAND_Y		(RH_MENU_ROW_TITLE_Y + RH_MENU_ROW_H)	/* 16 */
#define RH_MENU_ROW_SEA_Y		(RH_MENU_ROW_LAND_Y  + RH_MENU_ROW_H)	/* 32 */
#define RH_MENU_ROW_SKY_Y		(RH_MENU_ROW_SEA_Y   + RH_MENU_ROW_H)	/* 48 */

#define RH_MENU_ICON_X			(2)
#define RH_MENU_VEH_W			(CAR_SPRITE_W)
#define RH_MENU_VEH_H			(CAR_SPRITE_H)

#define RH_MENU_VEH_PY_OFFSET	(5)

#define RH_MENU_LABEL_X			(42)
#define RH_MENU_LABEL_COL_W		(128 - RH_MENU_LABEL_X - 2)

static uint8_t	rh_menu_focus = RH_MENU_ITEM_START;
static uint16_t rh_menu_tick  = 0;

static const char *const rh_menu_item_label[RH_MENU_ITEM_COUNT] = {
	"Start",
	"Record Score",
	"Setting",
};


#define RH_TITLE_TEXT	"ROAD HUNTER"
#define RH_TITLE_SIZE	(2)
#define RH_TITLE_GAP	(1)

static uint16_t rh_menu_tight_text_width(const char *s, uint8_t size, uint8_t gap) {
	uint16_t len = 0;
	while (*s++) {
		len++;
	}
	return (len > 0) ? (len * 5 * size + (len - 1) * gap) : 0;
}

static void rh_menu_tight_print(int16_t x, int16_t y, const char *s, uint8_t size, uint8_t gap) {
	while (*s) {
		view_render.drawChar(x, y, *s, WHITE, WHITE, size);
		x += 5 * size + gap;
		s++;
	}
}

static void rh_menu_row_title_display(void) {
	uint16_t w = rh_menu_tight_text_width(RH_TITLE_TEXT, RH_TITLE_SIZE, RH_TITLE_GAP);
	int16_t	 x = (128 - w) / 2;
	rh_menu_tight_print(x, RH_MENU_ROW_TITLE_Y + 1, RH_TITLE_TEXT, RH_TITLE_SIZE, RH_TITLE_GAP);
}


static void rh_menu_vehicle_thruster_display(int16_t px, int16_t py) {
	int by = py + (RH_MENU_VEH_H / 2) - 1;
	int bx = px - 1;
	int flicker = rh_menu_tick & 0x03;
	int flame_len = 3 + (flicker < 2 ? 1 : 0);

	view_render.fillTriangle(bx, by - 1, bx, by + 2, bx - flame_len, by, WHITE);

	int p0x = bx - flame_len - 2 - (int)(rh_menu_tick % 3);
	view_render.fillRect(p0x, by - 1, 2, 2, WHITE);

	int p1x = bx - flame_len - 6 - (int)(rh_menu_tick % 3);
	int p1y = by + ((rh_menu_tick / 2) % 3) - 1;
	view_render.drawPixel(p1x, p1y, WHITE);
	view_render.drawPixel(p1x + 1, p1y + 1, WHITE);
}

static void rh_menu_vehicle_cannon_display(int16_t px, int16_t py) {
	int gx = px + RH_MENU_VEH_W;
	int gy = py + (RH_MENU_VEH_H / 2) - 2;
	view_render.drawFastHLine(gx, gy, 5, WHITE);
	view_render.drawFastHLine(gx, gy + 1, 5, WHITE);
	view_render.fillRect(gx + 4, gy - 1, 2, 4, WHITE);
}

static void rh_menu_ripple_display(int cx, int cy, int phase) {
	int step = (rh_menu_tick + phase) % 12;
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

static void rh_menu_boat_hull_display(int16_t px, int16_t py) {
	int bx = px;
	int by = py + 1;			   /* deck line */
	int bb = py + RH_MENU_VEH_H;   /* hull bottom / waterline */

	view_render.fillRect(bx, by, RH_MENU_VEH_W - 4, bb - by, WHITE);
	view_render.fillTriangle(bx + RH_MENU_VEH_W - 4, by, bx + RH_MENU_VEH_W + 2, (by + bb) / 2,
		bx + RH_MENU_VEH_W - 4, bb, WHITE);
	view_render.fillRect(bx - 1, by + 1, 2, bb - by - 2, WHITE); /* transom stern */

	/* small pilot cabin amidships with a window slit */
	view_render.fillRect(bx + 3, by - 4, 6, 4, WHITE);
	view_render.drawFastHLine(bx + 4, by - 3, 4, BLACK);

	/* waterline ripples lapping at the hull */
	rh_menu_ripple_display(bx - 1, bb + 2, 0);
	rh_menu_ripple_display(bx + 9, bb + 2, 6);
}

/*****************************************************************************/
/* Row 1: land zone -- dashed road, normal player car + cannon.              */
/*****************************************************************************/
static void rh_menu_row_land_display(int16_t row_y) {
	int16_t px = RH_MENU_ICON_X;
	int16_t py = row_y + RH_MENU_VEH_PY_OFFSET;

	int line_y = py + RH_MENU_VEH_H - 1;
	for (int x = -(int)(rh_menu_tick % 20); x < 128; x += 20) {
		view_render.drawFastHLine(x, line_y, 10, WHITE);
	}

	rh_menu_vehicle_thruster_display(px, py);
	view_render.drawBitmap(px, py - 2, bitmap_car, CAR_SPRITE_W, CAR_SPRITE_H, WHITE);
	rh_menu_vehicle_cannon_display(px, py);
}

/*****************************************************************************/
/* Row 2: sea zone -- boat hull + mast/flag + cannon, riding on water        */
/* (ripples double as the water cue here since a full wave-grid line, like  */
/* the title screen's wider-spaced rows had, doesn't fit a 16px row).        */
/*****************************************************************************/
static void rh_menu_row_sea_display(int16_t row_y) {
	int16_t px = RH_MENU_ICON_X;
	int16_t py = row_y + RH_MENU_VEH_PY_OFFSET;

	rh_menu_vehicle_thruster_display(px, py);
	rh_menu_boat_hull_display(px, py);
	rh_menu_vehicle_cannon_display(px, py);

	/* mast + fluttering flag, identical to the sea-boss form */
	int deck_x = px + (RH_MENU_VEH_W / 2) - 3;
	int deck_y = py - 3; /* top of the pilot cabin */
	view_render.drawFastVLine(deck_x + 1, deck_y - 4, 4, WHITE);
	int flap = (rh_menu_tick / 4) % 2;
	view_render.drawLine(deck_x + 2, deck_y - 4, deck_x + 4 + flap, deck_y - 3, WHITE);
}

/*****************************************************************************/
/* Row 3: sky zone -- drifting cloud, player car + cannon + wings.           */
/*****************************************************************************/
static void rh_menu_row_sky_display(int16_t row_y) {
	int16_t px = RH_MENU_ICON_X;
	int16_t py = row_y + RH_MENU_VEH_PY_OFFSET;

	for (int x = -(int)(rh_menu_tick % 44); x < 128; x += 44) {
		view_render.drawFastHLine(x, py - 6, 6, WHITE);
		view_render.drawPixel(x + 8, py - 6, WHITE);
	}

	rh_menu_vehicle_thruster_display(px, py);
	view_render.drawBitmap(px, py - 2, bitmap_car, CAR_SPRITE_W, CAR_SPRITE_H, WHITE);
	rh_menu_vehicle_cannon_display(px, py);

	/* wings, identical to the sky-boss form */
	int cy = py + (RH_MENU_VEH_H / 2);
	view_render.fillTriangle(px + 3, cy, px - 5, cy - 6, px + 7, cy - 2, WHITE);
	view_render.fillTriangle(px + 3, cy, px - 5, cy + 6, px + 7, cy + 2, WHITE);
}

/*****************************************************************************/
/* Menu labels -- one per row, right next to that row's vehicle.            */
/*****************************************************************************/
static uint16_t rh_menu_text_width(const char *s, uint8_t size) {
	uint16_t len = 0;
	while (*s++) {
		len++;
	}
	return (len > 0) ? (len * 6 * size - size) : 0;
}

static void rh_menu_item_label_display(uint8_t index, int16_t row_y) {
	const char *label = rh_menu_item_label[index];

	/* Prefer the bigger size; fall back to size 1 if it can't fit in the
	   label column (e.g. "Record Score" at size 2 is 142px, wider than the
	   whole 128px screen let alone this column). */
	/* Use one font size for every menu item */
	uint8_t size = 1;
	uint16_t text_w = rh_menu_text_width(label, size);
	int16_t text_x = RH_MENU_LABEL_X;
	int16_t text_y = row_y + (RH_MENU_ROW_H - 8 * size) / 2;

	if (rh_menu_focus == index) {
		uint16_t box_w = text_w + 6 * size;
		int16_t	 box_x = text_x - 3 * size;
		view_render.drawRect(box_x, row_y, box_w, RH_MENU_ROW_H - 1, WHITE);
	}
	view_render.setTextSize(size);
	view_render.setCursor(text_x, text_y);
	view_render.print(label);
}

/*****************************************************************************/
static void view_scr_rh_menu() {
	view_render.setTextColor(WHITE);

	rh_menu_row_title_display();

	rh_menu_row_land_display(RH_MENU_ROW_LAND_Y);
	rh_menu_item_label_display(RH_MENU_ITEM_START, RH_MENU_ROW_LAND_Y);

	rh_menu_row_sea_display(RH_MENU_ROW_SEA_Y);
	rh_menu_item_label_display(RH_MENU_ITEM_RECORD, RH_MENU_ROW_SEA_Y);

	rh_menu_row_sky_display(RH_MENU_ROW_SKY_Y);
	rh_menu_item_label_display(RH_MENU_ITEM_SETTING, RH_MENU_ROW_SKY_Y);
}

view_dynamic_t dyn_view_rh_menu = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_rh_menu
};

view_screen_t scr_rh_menu = {
	&dyn_view_rh_menu,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void scr_rh_menu_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		screen_clear();
		rh_menu_focus = RH_MENU_ITEM_START;
		rh_menu_tick  = 0;
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_MENU_ANIM_TICK, AC_DISPLAY_MENU_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
	} break;

	case AC_DISPLAY_MENU_ANIM_TICK: {
		rh_menu_tick++;
	} break;

	case AC_DISPLAY_BUTON_UP_PRESSED: {
		if (rh_menu_focus > 0) {
			rh_menu_focus--;
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
		}
	} break;

	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		if (rh_menu_focus < RH_MENU_ITEM_COUNT - 1) {
			rh_menu_focus++;
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
		}
	} break;

	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		if (rh_sound_enabled) {
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_MENU_ANIM_TICK);
		screen_clear();
		switch (rh_menu_focus) {
		case RH_MENU_ITEM_START:
			SCREEN_TRAN(scr_road_hunter_handle, &scr_road_hunter);
			break;
		case RH_MENU_ITEM_RECORD:
			SCREEN_TRAN(scr_rh_record_handle, &scr_rh_record);
			break;
		case RH_MENU_ITEM_SETTING:
			SCREEN_TRAN(scr_rh_settings_handle, &scr_rh_settings);
			break;
		}
	} break;

	default:
		break;
	}
}