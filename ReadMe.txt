Project requires dx11 card. 

I have included the original paper "p471-muller.pdf" that the project is based on.

MeshDeformation Class is the main class that holds the implementation of the formulas. 
MatrixMath2 is where i tried to implement matrix math that is needed.This was before i started the implementation of Eigen3 math library. 
But some of the functions are still used from MatrixMath2, it also contains the main algorythm for matrix decomposition.

Controls:
AntTweakbar allows to change alpha and beta values. (alpha- stiffness, beta - deformation ammount).

NUMPAD_KEYS:
Move in specific axis.
Num_7 X+
Num_4 X-
Num_8 Y+
Num_5 Y-
Num_9 Z+
Num_6 Z-

Num_0 Add vertex to the sellected List.
Num_1 clear the sellection list.

Use mouse to hover over vertecies and press Num_0 to sellect the vertecie , after that(Num 7,4,8,5,9,6) can be used to move vertecies.
(the sellection method has been changed, and there is currently no visual representation where the sellection tool is(when the program is launched the tools is at the top left of the window. 
Moving the mouse will respond with the tool acordingly.)

Arrow_keys - Move the camera.

Number_Keys = 1-Shape matching. 2-Linear. 3-Quadratic.