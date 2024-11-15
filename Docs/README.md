# SD1 Assignment 5: Libra Foundation

## C34 Yu-Wei Tseng
Implementation of Libra, a semi-procedural action-arcade-RPG shooter.


## How to Use

### Keyboard
- **F1:** Toogles the debug render for entities.
- **F3:** Toggles “noclip” developer cheat, allowing the player to move through solid tiles.
- **F4:** Toogles the debug camera, which shows the entire current Map onscreen.
- **F8:** Does a “hard restart” of the game, deleting and re-newing the Game instance.
- **WASD:** To drive forward & turn toward direction.
- **IJKL:** To aim turret toward direction.
- **P:** To start new game, or toggle pause in-game.
- **ESC:** To pause & leave game, and again to quit.
- **T:** Slows game simulation time to 1/10 the normal rate.  
- **Y:** Speeds game simulation time up to 4x the normal rate.  
- **O:** Step single update and pause.

### Xbox controller
- **XBOX_BUTTON_DPAD_UP:** Toogles the debug render for entities.
- **XBOX_BUTTON_DPAD_DOWN:** Toggles “noclip” developer cheat, allowing the player to move through solid tiles.
- **XBOX_BUTTON_LSHOULDER:** Toogles the debug camera, which shows the entire current Map onscreen.
- **XBOX_BUTTON_DPAD_RIGHT:** Does a “hard restart” of the game, deleting and re-newing the Game instance.
- **XBOX_LEFT_TRIGGER:** To drive forward & turn toward direction.
- **XBOX_RIGHT_TRIGGER:** To aim turret toward direction.
- **XBOX_BUTTON_START:** To start new game, or toggle pause in-game.
- **XBOX_BUTTON_BACK:** To pause & leave game, and again to quit.
- **XBOX_BUTTON_RTHUMB:** Slows game simulation time to 1/10 the normal rate.  
- **XBOX_BUTTON_LTHUMB:** Speeds game simulation time up to 4x the normal rate.  
- **XBOX_BUTTON_RSHOULDER:** Step single update and pause.

## Known Issues
- Haven't implement the corrective physics for the entities.
- Debug render is slightly different than the demo.
- Turret acts weird while using xbox controller.

## Deep Learning
The first lesson I learned from this assignment is to start working on it as early as possible. 
I couldn't complete all the requirements because I was rushing to finish. 
However, I began by setting up the project structure and tried to make my code more readable by refactoring a large function into several smaller ones. 
I ensured that the dependencies between classes were appropriate. 
After this submission, I will definitely address some remaining issues in Libra, and, more importantly, I plan to go back to my engine code to make it more readable without having to open each code file. 
Another key takeaway is the importance of writing simple, easy-to-read code (even if it might seem simplistic), avoiding unnecessary use of templates and smart pointers. 
These are the main goals I aim to pursue as a programmer.
