CFLAGS		+= -I./sources/app/road_hunter_game
CPPFLAGS	+= -I./sources/app/road_hunter_game

VPATH += sources/app/road_hunter_game

# CPP source files
# Road Hunter game
SOURCES_CPP += sources/app/road_hunter_game/rh_game_core.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_player.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_enemy.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_obstacle.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_bullet.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_chest.cpp
SOURCES_CPP += sources/app/road_hunter_game/rh_game_boss.cpp
