#include "screens.h"
#include "rh_game_core.h"

#define RH_GAME_OVER_OPTION_RETRY   (0)
#define RH_GAME_OVER_OPTION_MENU    (1)

static uint8_t rh_game_over_focus = RH_GAME_OVER_OPTION_RETRY;


static void rh_game_over_draw_frame() {
	view_render.drawRect(0, 0, 127, 63, WHITE);
	view_render.drawRect(3, 3, 121, 57, WHITE);

	view_render.drawLine(0, 3, 3, 0, WHITE);		 /* top-left */
	view_render.drawLine(126, 0, 123, 3, WHITE);	 /* top-right */
	view_render.drawLine(0, 62, 3, 59, WHITE);		 /* bottom-left */
	view_render.drawLine(126, 62, 123, 59, WHITE);	 /* bottom-right */
}

static void view_scr_rh_game_over() {
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	rh_game_over_draw_frame();

	view_render.setCursor(34, 6);
	view_render.print("GAME OVER");
	view_render.drawFastHLine(9, 15, 110, WHITE);

	/* Score centered close below the divider */
	char buf[32];
	snprintf(buf, sizeof(buf), "Score:%d", (int)rh_score);
	int len = strlen(buf);
	int text_w = len * 6;
	int16_t x = (128 - text_w) / 2;
	view_render.setCursor(x, 19);
	view_render.print(buf);

	/* Record centered below Score */
	snprintf(buf, sizeof(buf), "Record:%d", (int)rh_high_score);
	len = strlen(buf);
	text_w = len * 6;
	x = (128 - text_w) / 2;
	view_render.setCursor(x, 28);
	view_render.print(buf);

	/* second divider separating the stats from the menu options */
	view_render.drawFastHLine(9, 36, 110, WHITE);

	/* Retry and Menu left-aligned below the second divider */
	for (uint8_t i = 0; i < 2; i++) {
		int16_t y = 39 + i * 10;
		if (rh_game_over_focus == i) {
			view_render.drawRect(6, y - 1, 116, 10, WHITE);
		}
		view_render.setCursor(12, y);
		if (i == RH_GAME_OVER_OPTION_RETRY) {
			view_render.print("Retry");
		}
		else {
			view_render.print("Menu");
		}
	}
}

view_dynamic_t dyn_view_rh_game_over = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	 },
	view_scr_rh_game_over
};

view_screen_t scr_rh_game_over = {
	&dyn_view_rh_game_over,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void scr_rh_game_over_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		screen_clear();
		rh_game_over_focus = RH_GAME_OVER_OPTION_RETRY;
	} break;

	case AC_DISPLAY_BUTON_UP_PRESSED: {
		if (rh_game_over_focus > RH_GAME_OVER_OPTION_RETRY) {
			rh_game_over_focus--;
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
		}
	} break;

	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		if (rh_game_over_focus < RH_GAME_OVER_OPTION_MENU) {
			rh_game_over_focus++;
			if (rh_sound_enabled) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
		}
	} break;

	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		if (rh_sound_enabled) {
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
		if (rh_game_over_focus == RH_GAME_OVER_OPTION_RETRY) {
			SCREEN_TRAN(scr_road_hunter_handle, &scr_road_hunter);
		}
		else {
			SCREEN_TRAN(scr_rh_menu_handle, &scr_rh_menu);
		}
	} break;

	default:
		break;
	}
}
