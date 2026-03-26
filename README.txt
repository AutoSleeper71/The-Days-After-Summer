An Explanation of the Source Code of Days After Summer
By iRobit



The days after summer is a narrative-driven game developed in C using the raylib graphics library. The game explores a story inspired by the five stages of grief, where each stage is represented by a different floor in an elevator. Each floor contains a short gameplay segment that reflects the emotional theme of that stage.

The game aims to demonstrate the experiences of someone going through a hard time in their life, and ends with coming to terms and moving on from their traumatic experience.

The game is built and edited using Visual Studio Code, with compilation done through the GCC compiler included in w64devkit.

Compilation Instructions
To compile and run the project, the following tools are required:
raylib – graphics/game development library


GCC – C compiler


w64devkit – MinGW-based development environment used with raylib


Visual Studio Code – code editor used during development

Build Instructions (Windows only)
Open a terminal in the project root directory.
Compile the project using the raylib toolchain:C:\raylib\w64devkit\bin\gcc.exe src/*.c -Iinclude -IC:/raylib/w64devkit/x86_64-w64-mingw32/include -LC:/raylib/w64devkit/x86_64-w64-mingw32/lib -lraylib -lopengl32 -lgdi32 -lwinmm -o game.exe
Run the game


Why we used the tools we did:
Raylib - a beginner friendly game development tool. We used it to add graphics rendering, input handling, sound playback and window management. Raylib allowed all this in a neat package, which is why we chose it over C libraries.

Visual Studio Code - this is a must for most C++ developers, especially amateurs to the programming space like us, as this allows us to write, compile, and debug code from one place. In addition, it provided syntax highlighting, debugging support and integrated terminal, which was a big help with debugging.

GCC / w64devkit - Compilation is handled by GCC, distributed through w64devkit. This toolchain is recommended for raylib because it ensures compatibility between the compiler and raylib libraries.

AI tools - we used chatgpt and github copilot to help with code generation. That of course, did not mean our work was not cut out for us, since these tools often made mistakes or did not generate the code exactly as we wanted it to, most of the time we would alter the code to suit the environment perfectly before adding it to our workspace.

Our Development Approach 
Because of the nature of our game, we structured it into modular components, with each major section of gameplay separated into its own source file.
Some examples include: 
elevator.c – elevator transitions between stages
evel1.c – gameplay for the first stage
level2.c – gameplay for the second stage
ending.c – ending sequence
This modular structure allows multiple team members to work on different systems simultaneously without interfering with each other's code.
We will go further into detail about the thinking behind our development at the “folder structure explanation” section.



Current Progress
Our implemented features include: 
A main menu - with options to start game, continue game and to exit the application.
Elevator animation - eye lids opening and closing, as well as a elevator shake.
Transition between multiple levels - Using GameStates to move between levels easily.
A dialogue system - a new dialogue system that makes it very easy to add more dialogue texts in the future, saving a lot of headaches.
Basic input handling - Use player input such as UP, DOWN, SPACE and ENTER to allow the player to progress through the game.


What we are working on now

Adding character avatars - adding character avatars to the bottom left of the dialogue box to make it easier to understand who is talking and make the game look more professional.
Sound effects - adding sound to every level in the game, making it more interactive and game-like.
Gameplay loops - add actual minigames to the game, to add some fun breaks in between the heavy story.
More dialogue options - add more dialogue texts into the game to fleshen out the story of the game
Good/bad endings - use the mini games to add ending features to the game.

Our Folder Structure
The project is organized into multiple directories to separate different parts of the game, making the code easier to understand and maintain. Our workspace is split into 3 separate folders: source code, include and assets folder:

The src folder contains all C source files (.c) that implement the game's functionality. Each file corresponds to a specific system or gameplay stage.
Examples:
main.c
 The entry point of the program. It initializes the game window, sets up the main game loop, and calls the main game logic functions.


game.c
 Handles the core game logic, such as managing the current state of the game (menu, gameplay, ending).


menu.c
 Implements the main menu where players can start the game or exit.


elevator.c
 Controls the elevator mechanic used to move between different floors, which represent the different stages of the story.


level1.c – level4.c
 Each file implements the gameplay logic for a specific level or stage.


ending.c
 Handles the ending sequence once the player has completed all stages.


The include folder contains all header files (.h), which define the functions, structures, and variables used across multiple source files.
Header files serve two main purposes:
Function declarations – allowing different source files to call functions from other modules.


Code organization – separating interface definitions from implementation.


For example:
game.h declares functions related to overall game logic.


menu.h declares menu-related functions.
level1.h declares functions specific to level 1 gameplay.

Assets Folder (assets)
The assets folder stores external resources used by the game, such as:
Images (sprites, backgrounds, UI elements)


Sound effects


Music
Fonts
Design Reasoning for This Structure
The project uses a modular structure where different parts of the game are separated into their own files and directories. This approach was chosen for several reasons:
Improved readability – smaller files are easier to understand.


Team collaboration – different developers can work on separate modules simultaneously.


Easier debugging – problems can be isolated to specific components.


Scalability – new levels or systems can be added without modifying existing files extensively.


Overview of the Program Structure

Your game is structured using a state-based system.
 This means the program is always in one of several states, such as:
Menu


Elevator


Level 1


Level 2


Level 3


Level 4


Ending


Each state has its own update function that runs every frame.

Program Execution Flow
1. Program Start
int main()
The program begins here.

2. Get Monitor Resolution

This gets the resolution of the monitor.
3. Create the Window

Creates the window where the game is drawn, allows sounds and music to work, and
The window switches to fullscreen mode.

6. Hide Mouse Cursor

Since the game is keyboard controlled, the cursor is hidden, and the game sets to run at 60 frames per second.

7. Load Assets

These functions load:
textures


sounds


backgrounds


8. Set Initial State
GameState state = MENU;
The game starts in the menu.

The Main Game Loop
This is the heart of the program.

The loop runs every frame until the user closes the game.
Each frame does:
Begin drawing


Update current game state


Draw graphics


End drawing




State Switch System

This checks which state the game is currently in.

MENU

The menu logic runs.
If the player presses start, the function returns:
ELEVATOR
Which changes the state.

ELEVATOR
state = UpdateElevator();

Handles:
dialogue


animations


level selection



LEVELS
state = UpdateLevel1();
state = UpdateLevel2();
...
Each level contains its own gameplay.

ENDINGS

Different endings depend on choices made earlier.

End Drawing

The frame is finished and shown on screen.
When the loop ends:
CloseWindow();
CloseAudioDevice();
The program safely closes.

2. elevator.c — Elevator Hub System
The elevator is the central hub between levels.
It handles:
intro animations


dialogue


level selection



Initialization

Runs once when the elevator loads.
It loads textures:

These are the background images.

Reset Animation Variables

These control the intro animation sequence.

Create Placeholder Avatar

Creates a temporary red portrait image.
This is used in dialogue boxes.


This function runs every frame while the player is in the elevator.

Step 1 — Screen Size

Used to position UI elements.

Step 2 — Animation Sequence
Before the player can choose levels, an animation plays.

There are 3 animation phases:

Phase 0 — Fade In

The screen fades from black to the background.
When finished:
animState = 1

Phase 1 — Eyelid Animation
Two rectangles close like eyelids:

This simulates blinking. It works because the two if statements keep going until the duration timer is up, which is 2 seconds total.

Phase 2 — Elevator Shake

The background shakes slightly to simulate elevator movement.
After 2 seconds:
animState = 3
Animations end.

Step 3 — Dialogue System
Dialogue initializes using:


Dialogue Case 0
Shows the first dialogue:

The dialogue function returns 0 when finished, advancing the state.

Once the player gets an option, this line plays:


Step 4 — Level Selection
After dialogue finishes:

Player can choose a level.

Navigation
UP / DOWN arrows
change:
selected_el

Drawing Level Buttons
Example:

Unlocked levels are shown.
Locked levels are hidden.


Selecting Level
ENTER
Returns the chosen state:
return LEVEL1

The main loop then switches state.

3. level1.c — First Level
This is currently a placeholder level.


Function

Runs every frame while in level 1.

Draw Text

Displays instructions.

Finish Level
If player presses space:

Then:
level2Unlocked = 1
This unlocks the next level.
And returns:
return ELEVATOR
Which sends the player back.

4. menu.c — Dialogue System

Dialogue Features
The system includes:
Animated Text

Characters appear gradually.

Every frame reveals one character.

Skip Text

Press:
ENTER or SPACE
to show the full text instantly.

Word Wrapping
Because the raylib version lacks DrawTextRec, the code manually wraps text.
It builds lines word by word and checks width using:




Dialogue Options
Players can select responses.

Navigation:
UP / DOWN

Selection:
ENTER
The function returns:
0, 1,2
depending on which option was chosen.

Global Game Variables (game.c)
These variables control overall game progress.

Next Level

Stores the next level to load.

Score Counters

These track choices affecting the ending.

Level Unlock System

Controls which levels appear in the elevator.


Resets the entire game state.
Used when restarting.
It:
resets scores


locks levels again


returns to Level1









Overall Execution Flow
When the game runs:
Start Program
    ↓
Initialize Window
    ↓
Load Assets
    ↓
Enter Game Loop
    ↓
MENU
    ↓
ELEVATOR (dialogue + animations)
    ↓
LEVEL1
    ↓
ELEVATOR
    ↓
LEVEL2
    ↓
LEVEL3
    ↓
LEVEL4
    ↓
ENDING
Our Current Limitations
It is not very out of place to say that currently the game has very little to actually do and interact with. We plan to add content to each of the 5 levels of the game, and we want to make sure that the outcome of each level matters and affects the ending the player receives.
We are also currently lacking assets, as for now we only have those that we were experimenting with. That of course is something we are working on, and we will hopefully have a very decent looking game by the end of the 6 weeks.
Another thing is that the dialogue is still a work in progress, and we hope to improve it in the near future.

Conclusion
We are overall quite satisfied with our development progress in the programming front. We have a very solid ground work and structure, so adding new functions and assets should be much easier because of this.
We will continue to develop our project by working on the limitations we mentioned previously, and are excited to see the development of the game through to the end. 
