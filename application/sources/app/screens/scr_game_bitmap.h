#ifndef __SCR_GAME_BITMAP_H__
#define __SCR_GAME_BITMAP_H__

#include "view_render.h"

/* Player car sprite 14x9 (upgraded battle car) */
#define CAR_SPRITE_W 14
#define CAR_SPRITE_H 9
extern const unsigned char PROGMEM bitmap_car[];

/* Enemy sprite 8x8 */
#define ENEMY_SPRITE_W 8
#define ENEMY_SPRITE_H 8
extern const unsigned char PROGMEM bitmap_enemy[];

/* Tank enemy sprite 8x8 (with turret) */
#define TANK_SPRITE_W 8
#define TANK_SPRITE_H 8
extern const unsigned char PROGMEM bitmap_tank[];

/* Obstacle (rock) sprite 8x8 */
#define OBSTACLE_SPRITE_W 8
#define OBSTACLE_SPRITE_H 8
extern const unsigned char PROGMEM bitmap_obstacle[];

/* Treasure chest sprite 8x8 */
#define CHEST_SPRITE_W 8
#define CHEST_SPRITE_H 8
extern const unsigned char PROGMEM bitmap_chest[];

/* Drone sprite 7x6 */
/* Drone sprite removed */

/* Explosion sprite 8x8 */
#define EXPLOSION_SPRITE_W 8
#define EXPLOSION_SPRITE_H 8
extern const unsigned char PROGMEM bitmap_explosion[];

#endif	  //__SCR_GAME_BITMAP_H__
