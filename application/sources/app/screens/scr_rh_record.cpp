#include "screens.h"
#include "rh_game_core.h"

static void view_scr_rh_record() {
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(10, 10);
	view_render.print("Record Score");

	view_render.setCursor(10, 28);
	view_render.print("High Score:");
	view_render.print((int)rh_high_score);

	view_render.setCursor(10, 44);
	view_render.print("Press MODE to go");
	view_render.setCursor(10, 54);
	view_render.print("back");
}

view_dynamic_t dyn_view_rh_record = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	 },
	view_scr_rh_record
};

view_screen_t scr_rh_record = {
	&dyn_view_rh_record,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void scr_rh_record_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		SCREEN_TRAN(scr_rh_menu_handle, &scr_rh_menu);
	} break;

	default:
		break;
	}
}
