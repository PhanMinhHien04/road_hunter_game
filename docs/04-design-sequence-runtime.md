<h1 align="center">Runtime Signal Processing</h1>

This document explains how the Road Hunter game processes button input, timer signals, game-loop updates, and state transitions. Road Hunter operates under the AK event-driven task architecture, but simplifies entity execution: instead of running multiple asynchronous tasks for each game object, all updates and collision tests run synchronously within the display task (`AC_TASK_DISPLAY_ID`) on a periodic game tick.

---

## I. Overview

The Road Hunter application is centered around the display task (`AC_TASK_DISPLAY_ID`), which controls screen-level transitions, button events routing, and periodic display refreshes.

**The Game Loop Timer:**
- Signal: `AC_DISPLAY_RH_GAME_TICK`
- Task Destination: `AC_TASK_DISPLAY_ID`
- Rate: `AC_DISPLAY_RH_GAME_TICK_INTERVAL = 120 ms` (approx. 8.3 ticks per second)

**Main Runtime Flow:**
1. Hardware interrupts (buttons, timers) register signals into the AK message pool.
2. The AK scheduler dispatches messages to the display task handler.
3. For user buttons (UP/DOWN), the handler calls player movement functions immediately to change lanes.
4. On `AC_DISPLAY_RH_GAME_TICK`, the screen handler calls the game loop update function `rh_game_update()` to advance coordinates, spawn hazards, autoshoot bullets, and check collisions in a single synchronous pass.
5. Finally, the display manager draws the active screen, writing pixels to the OLED buffer and pushing the frame.

---

## II. High Level Architecture

### 1. Game Start Sequence
When the screen manager switches focus to the Road Hunter gameplay screen, it sends a `SCREEN_ENTRY` signal to `scr_road_hunter_handle()`. This clears the screen, resets variables, and arms the 120ms periodic tick timer.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    participant Manager as Screen Manager
    participant Scr as Screen Handler
    participant Q as AKOS Message Queue
    participant Core as Game Core
    participant Tmr as Timer Module

    Manager->>Scr: SCREEN_ENTRY (Dispatched to scr_road_hunter_handle)
    activate Scr
    Note over Scr: Call screen_clear()
    Scr->>Core: rh_game_reset_round()
    activate Core
    Note over Core: Reset score, timer, speed<br/>Call sub-reset functions: player, enemy,<br/>obstacle, bullet, chest, boss
    Core-->>Scr: 
    deactivate Core
    Scr->>Tmr: timer_remove_attr(AC_DISPLAY_SCREEN_IDLE)
    Scr->>Tmr: timer_set(AC_DISPLAY_SCREEN_IDLE, 12000 ms, ONE_SHOT)
    Scr->>Tmr: timer_set(AC_DISPLAY_RH_GAME_TICK, 120 ms, PERIODIC)
    deactivate Scr

    Note over Tmr: 120 ms later
    Tmr-)Q: Post AC_DISPLAY_RH_GAME_TICK
    Q-)Scr: Dispatch signal
```

<p align="center"><strong>*Figure 1:*</strong> Game start runtime sequence</p>

---

### 2. Game Playing Loop
During gameplay, user movement is processed instantly upon receiving button pressed signals. The main game tick updates coordinates and runs collision detection synchronously.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    actor Btn as Buttons
    participant Q as AKOS Message Queue
    participant Scr as Screen Handler
    participant Play as Player Module
    participant Core as Game Core
    participant Bul as Bullet Module
    participant En as Enemy Module

    Note over Btn,Q: User moves player (Asynchronous)
    Btn->>Q: Press UP/DOWN
    Q->>Scr: Dispatch AC_DISPLAY_BUTON_UP/DOWN_PRESSED
    activate Scr
    Scr->>Play: rh_game_player_move_up/down()
    Note over Scr: Play click sound
    deactivate Scr

    Note over Q: Periodic Game Tick (Every 120 ms)
    Q->>Scr: Dispatch AC_DISPLAY_RH_GAME_TICK
    activate Scr
    opt State == PLAYING
        Scr->>Core: rh_game_update()
        activate Core
        Note over Core: Update difficulty, speed & empower timers
        alt Boss Active
            Note over Core: Run rh_game_boss_update()
            Core->>Bul: rh_game_bullet_autoshoot()
            Core->>Bul: rh_game_bullet_update()
            Core->>Bul: rh_game_boss_check_bullet_hits()
        else Normal Road
            Note over Core: Spawn entities (enemies, obstacles, chests)
            Core->>En: rh_game_enemy_update() (checks collisions)
            Note over Core: Update obstacles, chests, lasers
            Core->>Bul: rh_game_bullet_autoshoot()
            Core->>Bul: rh_game_bullet_update()
        end
        Note over Core: Update explosion frames & increment tick
        Core-->>Scr: 
        deactivate Core
    end
    deactivate Scr
```

<p align="center"><strong>*Figure 2:*</strong> Gameplay loop tick sequence</p>

---

### 3. Game Over & Reset Sequence
If an enemy or obstacle collides with the player car without a shield/empowerment buff, the core invokes `rh_game_trigger_over()`. This plays a falling text animation before transferring control to the summary screen.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    participant Core as Game Core
    participant Scr as Screen Handler
    participant Q as AKOS Message Queue
    participant Tmr as Timer Module
    participant GameOver as Game Over Screen

    Note over Core: Collision detected during rh_game_enemy_update()
    Core->>Core: rh_game_trigger_over()
    Note over Core: Set state = GAME_OVER<br/>Set rh_state_timer = 24 ticks (2.8s)<br/>Play BUZZER_SOUND_LOWSCORE
    
    loop During next 24 tick signals
        Q->>Scr: Dispatch AC_DISPLAY_RH_GAME_TICK
        activate Scr
        Note over Scr: Decrement rh_state_timer<br/>Draw "GAME OVER" falling text animation
        deactivate Scr
    end

    Note over Q: Once rh_state_timer hits 0
    Q->>Scr: Dispatch AC_DISPLAY_RH_GAME_TICK
    activate Scr
    Scr->>Tmr: timer_remove_attr(AC_DISPLAY_RH_GAME_TICK)
    Scr->>GameOver: SCREEN_TRAN(scr_rh_game_over_handle, &scr_rh_game_over)
    deactivate Scr
```

<p align="center"><strong>*Figure 3:*</strong> Game over and transition sequence</p>

---

## III. Button Event Routing

Button callbacks are registered globally in the Board Support Package (`app_bsp.cpp`) and post events to the display task (`AC_TASK_DISPLAY_ID`). The active screen handler intercepts these signals and runs local code.

| Button Callback | Posted Signal | Target Task | Action in Main Screen (`scr_road_hunter.cpp`) | Action in Game Over Screen (`scr_rh_game_over.cpp`) |
|---|---|---|---|---|
| **UP Pressed** | `AC_DISPLAY_BUTON_UP_PRESSED` | `AC_TASK_DISPLAY_ID` | Moves player up one lane: `rh_game_player_move_up()`. | Moves selection focus up. |
| **DOWN Pressed** | `AC_DISPLAY_BUTON_DOWN_PRESSED` | `AC_TASK_DISPLAY_ID` | Moves player down one lane: `rh_game_player_move_down()`. | Moves selection focus down. |
| **MODE Pressed** | `AC_DISPLAY_BUTON_MODE_PRESSED` | `AC_TASK_DISPLAY_ID` | *Ignored.* Mode button is instead queried directly in the game loop. | Activates highlighted option (Retry or Menu). |

> **Note on Mode Button:** To support hold-to-fire and Gatling continuous fire, the Mode button is read directly as a raw hardware input (`io_button_mode_read()`) inside the game loop on every tick rather than acting on asynchronous button signals.

---

## IV. Task Ownership

All game variables and modules are executed within the context of the display task `AC_TASK_DISPLAY_ID`.

| Module | Data Owned | Functions Called |
|---|---|---|
| **Screen Handler** | `rh_game_tick`, `rh_state_timer` | `rh_game_reset_round()`, `rh_game_update()` |
| **Player Module** | `rh_player_lane` | `rh_game_player_move_up()`, `rh_game_player_move_down()`, `rh_game_player_reset()` |
| **Bullet Module** | `rh_player_bullets[]`, `rh_enemy_bullets[]`, `rh_explosions[]`, `rh_shoot_timer` | `rh_game_bullet_autoshoot()`, `rh_game_bullet_update()`, `rh_game_bullet_reset()` |
| **Enemy Module** | `rh_enemies[]`, `rh_enemy_kill_counter` | `rh_game_enemy_update()`, `rh_game_enemy_reset()`, `rh_game_enemy_spawn()` |
| **Obstacle Module** | `rh_obstacles[]` | `rh_game_obstacle_update()`, `rh_game_obstacle_reset()`, `rh_game_obstacle_spawn()` |
| **Chest Module** | `rh_chests[]`, `rh_active_power`, `rh_power_timer`, `rh_shield_active` | `rh_game_chest_update()`, `rh_game_chest_reset()`, `rh_game_chest_spawn()` |
| **Boss Module** | `rh_boss`, `rh_boss_projectiles[]`, `rh_boss_countdown` | `rh_game_boss_update()`, `rh_game_boss_reset()`, `rh_game_boss_start_next()` |
