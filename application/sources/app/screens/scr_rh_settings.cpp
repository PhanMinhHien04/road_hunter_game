#include "screens.h"
#include "rh_game_core.h"

static void view_scr_rh_settings() {
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(10, 10);
	view_render.print("Settings");

	view_render.setCursor(10, 28);
	view_render.print("Sound:");
	view_render.print(rh_sound_enabled ? "On" : "Off");

	view_render.setCursor(10, 44);
	view_render.print("MODE: toggle");
	view_render.setCursor(10, 54);
	view_render.print("UP/DOWN: back");
}

view_dynamic_t dyn_view_rh_settings = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	 },
	view_scr_rh_settings
};

view_screen_t scr_rh_settings = {
	&dyn_view_rh_settings,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void scr_rh_settings_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		rh_toggle_sound();
		if (rh_sound_enabled) {
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
	} break;

	case AC_DISPLAY_BUTON_UP_PRESSED:
	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		SCREEN_TRAN(scr_rh_menu_handle, &scr_rh_menu);
	} break;

	default:
		break;
	}
}
