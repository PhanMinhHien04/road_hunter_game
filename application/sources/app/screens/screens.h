#ifndef __SCREENS_H__
#define __SCREENS_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include <math.h>
#include <vector>

#include "qrcode.h"
#include "screens_bitmap.h"

// scr_idle
extern view_dynamic_t dyn_view_idle;
extern view_screen_t scr_idle;
extern void scr_idle_handle(ak_msg_t* msg);



// scr_startup
extern view_dynamic_t dyn_view_startup;
extern view_screen_t scr_startup;
extern void scr_startup_handle(ak_msg_t* msg);

// scr_rh_title
extern view_dynamic_t dyn_view_rh_title;
extern view_screen_t scr_rh_title;
extern void scr_rh_title_handle(ak_msg_t* msg);

// scr_road_hunter
extern view_dynamic_t dyn_view_road_hunter;
extern view_screen_t scr_road_hunter;
extern void scr_road_hunter_handle(ak_msg_t* msg);

// scr_rh_menu
extern view_dynamic_t dyn_view_rh_menu;
extern view_screen_t scr_rh_menu;
extern void scr_rh_menu_handle(ak_msg_t* msg);

// scr_rh_record
extern view_dynamic_t dyn_view_rh_record;
extern view_screen_t scr_rh_record;
extern void scr_rh_record_handle(ak_msg_t* msg);

// scr_rh_settings
extern view_dynamic_t dyn_view_rh_settings;
extern view_screen_t scr_rh_settings;
extern void scr_rh_settings_handle(ak_msg_t* msg);

// scr_rh_game_over
extern view_dynamic_t dyn_view_rh_game_over;
extern view_screen_t scr_rh_game_over;
extern void scr_rh_game_over_handle(ak_msg_t* msg);

// helper
extern void screen_clear(void);

#endif //__SCREENS_H__
