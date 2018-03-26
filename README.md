___________________________
|    BENCHMARK SOFTWARE   |
|                         |
|          for            |
|                         |
| ADAPTATIVE TESSELATION  |
|         METHODS         |
|_________________________|

Work done by :
------------------
ALMYRE Guillaume
CELOR Pierre
CHAMBRES Gaëtan
CHEVRIER Valentin
------------------

-----------------
I - Introduction
-----------------

The goal of this project is to compare the performances of the hardware tessellation method versus a custom method based on instanciation. You will be able to tweak lots of parameters like light, texture, LOD level, type of displacement, distance fog, etc... and visualize the performances of the system in the case you choose.

In this README you will find instructions for using the software, the commands you can use and details about the content of the folder.

------------------
II - Instructions
------------------

If you've found this README, you're at the right place.
In the same folder you will find a file named "install.sh".
Run this script. It will create a "build" folder and install the software inside. When the installation is over, you just have to go into the build fodler and run the executable.

---------------
III - Controls
---------------

  About the window:

On the software window, you will find all the parameters on the right panel.

If you want to upload a new heightmap, a new texture or quit, you have to use the menu on the top left corner :

  File -> Load heightmap
  or
  File -> Load texture
  or
  File -> Exit


  About the keyboard :

You can move the camera into the scene using the the directional arrows or the following letters (AZERTY system):
Z - front
S - back
Q - left
D - right

You can use "G" to active the "ground camera" (the camera will be following the relief when moving).

You can use "R" to reload the shaders corresponding to the scene.

You can use "B" to active the benchmark window.

--------------------
IV - Folder content
--------------------

In this folder, you will find 4 folders and 3 files.

As files, you will find :

  "README.md"      -   The README you are reading right now.
  "CmakeLists.txt"  -   The file needed for a correct install.
  "install.sh"      -   the script to run for installing the software.

As folders, you will find :

  "data"  -   In this folder you will find 3 sub-folders :
  |_____  "heightmaps"
  |_____  "shaders"
  |_____  "textures"    


            "heightmaps"  -   In this folder, you will find some default heightmaps you can open in the software. Obviously, you can upload any heightmap you want from your computer.

            "shaders"     -   in this folder, you will find the different shaders used by the software, fragment shaders, vertex shaders, the tessellation shaders for the hardware tessellation and custom shaders for the tessellation using instanciation.

            "textures"     -   In this folder, you will find some default textures you can apply to your heightmap. Obviously, you can upload any texture you want from your computer.


  "ext"     -  In this folder you will find 3 sub-folders :
  |______  "CImg"
  |______  "eigen3"
  |______  "surface_mesh"


            "CImg"          -  This library is used by the software to preprocess the heigtmap as an image.

            "eigen3"        -   The Eigen library is used for vectors and matrices computation.

            "surface_mesh"  -   This library is used for the mesh construction to render a 3D scene.


  "include"   -   In this folder, you will find all the header files relative to the implementation.

  "src"       -   in this folder you will find all the files containing the implementation of the software.
