CSCI 441 Computer Graphics, Fall 2025

Project: MP - The Alchemist's Guild

Land Name: Mystical Magical Planet
Land Members: Farina Meller, Wilfred, Elster, Tympanius

Description:
This program has the 4 heroes, all controllable, with an arcball and first
person camera that follows each of them. The arcball camera can be switched to
a free camera, independent of any hero. The heroes are in a land with a pretty
sky and a lush forest. There is a blue spotlight and red point light near the
center of the land.

Usage:
W - Move hero forward (arcball), Rotate camera up (free cam)
A - Rotate hero counter-clockwise (arcball), Rotate camera left (free cam)
S - Move hero backward (arcball), Rotate camera down (free cam)
D - Rotate hero clockwise (arcball), Rotate camera right (free cam)
Enter - Switch hero
Left click & drag mouse - Rotate camera
Shift & Left click & drag mouse - Zoom arcball camera
1 - Switch to arcball camera
2 - Switch to free camera
Space - Move forward in free camera
Shift & Space - Move backward in free camera

Compiling:
First, open a terminal and cd into the src directory.
Then, run "cmake . && make"
To run the program, run "./mp"

On Windows: Update CMakeLists.txt to your include directory.

Distribution of responsibilities:
Marina: Provided base code, implemented skybox, directional light, refactoring
James: Set up GitHub repo, implemented Wilfred, spotlight, point light,
Kaelem: Implemented Elster, free camera, first person camera, lots of Git help
Mark: Implemented Tympanius, trees, bushes, ground texture, ambient

This assignment took us 25+ hours over 3 weeks.
The labs helped us quite a bit with many aspects of the assignment.
The assignment got pretty frustrating at times, but seeing it all come together
in the end was worth it. 7/10
