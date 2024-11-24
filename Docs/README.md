# SD1 Assignment 6: Libra Prototype

## C34 Yu-Wei Tseng

Implementation of Libra, a semi-procedural action-arcade-RPG shooter.

## How to Use

### Keyboard

- **F1:** Toggles the debug render for entities.
- **F3:** Toggles noclip developer cheat, allowing the player to move through solid tiles.
- **F4:** Toggles the debug camera, which shows the entire current Map onscreen.
- **F8:** Does a hard restart of the game, deleting and re-newing the Game instance.
- **WASD:** To drive forward & turn toward direction.
- **IJKL:** To aim turret toward direction.
- **P:** To start new game, or toggle pause in-game.
- **ESC:** To pause & leave game, and again to quit.
- **T:** Slows game simulation time to 1/10 the normal rate.
- **Y:** Speeds game simulation time up to 4x the normal rate.
- **O:** Step single update and pause.

### Xbox controller

- **XBOX_BUTTON_DPAD_UP:** Toggles the debug render for entities.
- **XBOX_BUTTON_DPAD_DOWN:** Toggles noclip developer cheat, allowing the player to move through solid tiles.
- **XBOX_BUTTON_LSHOULDER:** Toggles the debug camera, which shows the entire current Map onscreen.
- **XBOX_BUTTON_DPAD_RIGHT:** Does a hard restart of the game, deleting and re-newing the Game instance.
- **XBOX_LEFT_TRIGGER:** To drive forward & turn toward direction.
- **XBOX_RIGHT_TRIGGER:** To aim turret toward direction.
- **XBOX_BUTTON_START:** To start new game, or toggle pause in-game.
- **XBOX_BUTTON_BACK:** To pause & leave game, and again to quit.
- **XBOX_BUTTON_RTHUMB:** Slows game simulation time to 1/10 the normal rate.
- **XBOX_BUTTON_LTHUMB:** Speeds game simulation time up to 4x the normal rate.
- **XBOX_BUTTON_RSHOULDER:** Step single update and pause.

## Known Issues

- When in noclip mode, if the player tries to shoot while in a tile, an error window will pop up.

## Deep Learning

Just like the last assignment, I think the most important thing is to write code that readers can easily understand. 
So, in this assignment, I started to ask myself if the code is easy to understand or not. 
I also went back to the previous engine code and tried to write some comments on it, so that I don't have to go all the way back to the engine code and rewrite it. 
Adding comments and renaming functions can actually help to avoid potential bugs. 
Regarding the engine code, the thing that bothered me was deciding which functions should be moved to the entity class. 
The game state, which we have to go back and forth with, definitely needs to be very clear to avoid unexpected bugs in the future.
