#include "button.h"

#include "sys_dbg.h"

#include "app.h"
#include "app_bsp.h"
#include "app_dbg.h"
#include "app_if.h"

#include "task_list.h"
#include "task_list_if.h"
#include "timer.h"
#include "screen_manager.h"
#include "scr_road_hunter.h"

button_t btn_mode;
button_t btn_up;
button_t btn_down;

static uint32_t app_get_idle_interval(void) {
	return AC_DISPLAY_SCREEN_IDLE_INTERVAL;
}

void btn_mode_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[btn_mode_callback] BUTTON_SW_STATE_PRESSED\n");
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_MODE_PRESSED);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, app_get_idle_interval(), TIMER_ONE_SHOT);
	}
		break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[btn_mode_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
	}
		break;

	default:
		break;
	}
}

void btn_up_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[btn_up_callback] BUTTON_SW_STATE_PRESSED\n");
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_UP_PRESSED);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, app_get_idle_interval(), TIMER_ONE_SHOT);
	}
		break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[btn_up_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
	}
		break;

	default:
		break;
	}
}

void btn_down_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[btn_down_callback] BUTTON_SW_STATE_PRESSED\n");
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_DOWN_PRESSED);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SCREEN_IDLE, app_get_idle_interval(), TIMER_ONE_SHOT);
	}
		break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[btn_down_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
	}
		break;

	default:
		break;
	}
}
