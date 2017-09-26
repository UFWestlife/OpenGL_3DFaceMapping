NOTE: It's tested that in Windows (or maybe other OS) launching the program takes a while. It's mainly because the process of loading obj file.

FUNCTIONS:
1 - Enable face model 1.
2 - Enable face model 2.
R - Reset state
	Note: 1,2 and R function loads the face model so it may takes time (in Windows especially).
Arrow Keys (Up, Down, Left, Right) - Rotate camera
F - Show/Hide face model. This is quick because it's not loading .obj file again.
C - Enable control mesh and texture. Press C twice to hide control mesh but keep textures. Press C again to disable.
A - Automatically fit the control points to the face model, using ray casting.
	Notes for this function:
	1. This function loads the face model again so it may takes time (in Windows especially).
	2. To have a better result, the control mesh is first placed in a cylinder shape before the ray casting.
D - Enable/Disable smooth surface, calculated by subdivision.
S - Save the position of control vertices to file "cm.p3".
L - Load the position of control vertices from file "cm.p3".
H - Enable/Disable hair model.
, (Comma) - Activate a animataion to a emotional face (i.e. smile)
. (Period) - Activate another animataion to a emotional face (i.e. frown)