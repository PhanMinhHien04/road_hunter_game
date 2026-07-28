#ifndef __SCR_ROAD_HUNTER_H__
#define __SCR_ROAD_HUNTER_H__

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

#include "screens.h"
#include "scr_game_bitmap.h"

#include "rh_game_core.h"
#include "rh_game_player.h"
#include "rh_game_enemy.h"
#include "rh_game_obstacle.h"
#include "rh_game_bullet.h"
#include "rh_game_chest.h"
#include "rh_game_boss.h"

extern view_dynamic_t dyn_view_road_hunter;
extern view_screen_t scr_road_hunter;
extern void scr_road_hunter_handle(ak_msg_t *msg);

#endif //__SCR_ROAD_HUNTER_H__
