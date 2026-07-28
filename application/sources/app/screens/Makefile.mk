CFLAGS		+= -I./sources/app/screens
CPPFLAGS	+= -I./sources/app/screens

VPATH += sources/app/screens

# CPP source files
SOURCES_CPP += sources/app/screens/screens_bitmap.cpp
SOURCES_CPP += sources/app/screens/scr_idle.cpp
SOURCES_CPP += sources/app/screens/scr_startup.cpp
SOURCES_CPP += sources/app/screens/scr_rh_title.cpp
SOURCES_CPP += sources/app/screens/scr_road_hunter.cpp
SOURCES_CPP += sources/app/screens/scr_rh_menu.cpp
SOURCES_CPP += sources/app/screens/scr_rh_record.cpp
SOURCES_CPP += sources/app/screens/scr_rh_settings.cpp
SOURCES_CPP += sources/app/screens/scr_rh_game_over.cpp
SOURCES_CPP += sources/app/screens/scr_game_bitmap.cpp
