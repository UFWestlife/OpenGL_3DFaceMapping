NOTE: It's tested that in Windows (or maybe other OS) launching the program takes a while. It's mainly because the process of loading obj file. <br>

FUNCTIONS: <br>
1 - Enable face model 1. <br>
2 - Enable face model 2. <br>
R - Reset state <br> 
Note: 1,2 and R function loads the face model so it may takes time (in Windows especially). <br>
Arrow Keys (Up, Down, Left, Right) - Rotate camera <br>
F - Show/Hide face model. This is quick because it's not loading .obj file again. <br>
C - Enable control mesh and texture. Press C twice to hide control mesh but keep textures. Press C again to disable. <br>
A - Automatically fit the control points to the face model, using ray casting. <br>
　　　　Notes for this function:<br>
　　　　1. This function loads the face model again so it may takes time (in Windows especially). <br>
　　　　2. To have a better result, the control mesh is first placed in a cylinder shape before the ray casting. <br>
D - Enable/Disable smooth surface, calculated by subdivision. <br>
S - Save the position of control vertices to file "cm.p3". <br>
L - Load the position of control vertices from file "cm.p3". <br>
H - Enable/Disable hair model. <br>
, (Comma) - Activate a animataion to a emotional face (i.e. smile) <br>
. (Period) - Activate another animataion to a emotional face (i.e. frown) <br>
