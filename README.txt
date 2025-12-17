CSCI 441 Computer Graphics, Fall 2025

Project: FP - The Metals

Land Name: Spooky Scary Maze
Land Members: Farina Meller, Wilfred, Elster, Tympanius

Description:
This program has the 4 heroes as evil enemies.
You control a person with a floating hand, using a first person camera and FPS controls.
You and the enemies are in a land with a maze made of large brick walls.
The enemies have point lights at their eyes, causing them to emanate scary red light.
Your character holds a flashlight (spotlight) while navigating the maze.
You also have a minimap to be able to see where the enemies are in relation to your position.
There is also a killer ball obstacle that follows a parameterized bezier curve.
Additionally, whenever the character gets close to an enemy, there is a scary chromatic aberration effect.

Usage:
W - Move the character forward with respect to where you're facing.
A - Strafe the character to the left with respect to where you're facing.
S - Move the character backwards with respect to where you're facing.
D - Strafe the character to the right with respect to where you're facing.
H - Hide the minimap
Z - (secret)
Mouse Movement: Move the camera

Compiling:
First, open a terminal and cd into the src directory.
Then, run "cmake . && make"
To run the program, run "./FP"

On Windows: Update CMakeLists.txt to your include directory.

Distribution of responsibilities:
Marina: Implemented Farina enemy code, Implemented Bezier Curve with arc length parameterization, refactoring.
James: Implemented Wilfred enemy code, spotlight, hand animation, wall collision, chromatic aberration.
Kaelem: Provided base code, Implemented Elster enemy code, first person camera, point light, minimap.
Mark: Implemented Tympanius enemy code, wall texture, ground texture, ambient lighting, minimap fixes

This assignment took us 25+ hours over 1 weeks.
The labs helped us quite a bit with many aspects of the assignment.
The assignment got pretty frustrating at times, but seeing it all come together
in the end was worth it. 8/10
