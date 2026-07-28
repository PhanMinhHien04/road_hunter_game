#include "rh_game_player.h"

#include "rh_game_core.h"

uint8_t rh_player_lane = ROAD_LANE_COUNT - 1;

void rh_game_player_reset(void) {
	rh_player_lane = ROAD_LANE_COUNT - 1; /* lane 5 (bottom lane), matches the menu preview */
}

void rh_game_player_move_up(void) {
	if (rh_player_lane > 0) {
		uint8_t next_lane = rh_player_lane - 1;
		if (rh_game_can_move_to_lane(next_lane)) {
			rh_player_lane = next_lane;
		}
	}
}

void rh_game_player_move_down(void) {
	if (rh_player_lane < ROAD_LANE_COUNT - 1) {
		uint8_t next_lane = rh_player_lane + 1;
		if (rh_game_can_move_to_lane(next_lane)) {
			rh_player_lane = next_lane;
		}
	}
}