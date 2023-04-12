# Toolpath_Gcode_generation
Simple example which contains the different toolpath generation, and G-code generation for the 5AXISMAKER machine

# Toolpath Generation and CNC Simulation



## Installation

Please compile the code with QMake file "TpGene_and_cncSim.pro".

**Platform**: 
	Windows + Visual Studio + QT-plugin 
	(tested QT version: 5.12.3 + msvc2017_64)

**Install Steps**: 
- **Install Visual Studio Extension plug-in (QT VS Tool)** to open the .pro file and generate the project
- **Set 'shapeLab' as the start up project**
- **Enable OpenMP to get best performace** at: ShapeLab Project Property -> 'Configuration Proerties' -> c/c++ -> Language -> Open MP Support -> Select 'Yes (/openmp)'
- **Open Console** at: ShapeLab Project Property -> 'Configuration Proerties' -> Linker -> System -> Select 'Console (/SUBSYSTEM:CONSOLE)' in 'SubSystem'

## Usage

**Step 0: input layer files (.obj) .**
- The files of the layers (.obj) for the model that is going to print need to be saved in the following folder
'**./3D_Print\Toolpath_Gcode_generation-main\DataSet\CURVED_LAYER**'

**Step 1:  Read Layers .**
Click button '**1. Read Layers**'.

- The layers will be loaded and form a whole model to display in the window automatically .

**Step 2:  3D coordinates modification** 
Input the needed value in the boxs for modification 
Click button '**2.Update Pos and Ort**'.

- The initially loaded model might be not in the appropriate position or orientation. The model should be upward along the Z-axis and its center should also be around the Z-axi.

- The boxes for **Xm**, **Ym** and **Zm** means the 'move' along the corresponding axis for the input milimeters.

- The boxes for **Xr**, **Yr** and **Zr** means the 'rotation' around the corresponding axis for the input degree angle.

**for 'Display'**

- By inputting the number of layer in the 'show' box, It will display the layers from 0 to the inputted number.

- By ticking the checkbox **each**, it will only display the individual layer of the inputted number.

- By clicking the button ** All**, it will display the whole model.

**Step 3:  Toolpath Generation**

input **width** and **distance** in the boxes 

Choosing and Clicking button 
'**3. 1 	Spiral Tp Generation**' or
'**3. 2 	Contour Tp Generation**' or
'**3. 3 	Zigzag Tp Generation**' 

- the 'width' will set the distance between the adjacent toolpathes.

- the 'distance' will set the distance between the adjacent nodes along the path.

- the '**Spiral**', '**Contour**' and '**Zigzag**' are 3 different patterns for the generated toolpath.

**Step 4: CNC Motion Planning**
- In the '**Model**' box, input the folder name of the needed model, which is stored at :
 '\Toolpath_Gcode_generation-main\DataSet\FABRICATION'
 
- Click the '**1. Read data**' Button,
the programme will automatically load the  Waypoints and Slices (Obj.) files.

- Input the needed layers and 'toolLns'

- Click the '**2. Calculate Extrusion Volume**',
The volume of the material will be calculated base on the waypoint distance and height.

- Click the '** 3. Singularity Optimization**',
It will calculate the movements of the 5axis and verify the 'PosNor' to reduce the effect of Singularity.

**Step 5: generate G-code file** 
Clicking button '**5. G Code Writing**'. 

- The G code will be stored at folder 
'**\Toolpath_Gcode_generation-main\DataSet\G_CODE**'

**Step 6: simulate the motion of printing** 
Clicking button '**Simulation**'.

- The **progress bar** shows the percentage of completed print volume, and clicking the checkbox '**stop**' will stop the simulation from running.

## Curved Layer Generation Algorithm

- The inputed layers and waypoints are generated from this paper ([*ACM Transactions on Graphics (Proceedings of SIGGRAPH Asia 2020)*, vol.39, no.6, article no.204, 2020.](https://dl.acm.org/doi/abs/10.1145/3414685.3417834)) , In specifically, this repository is supporting the part "Fabrication Enabling" of above TOG paper.([Source Code](https://github.com/GuoxinFang/ReinforcedFDM)),([Project Page](https://guoxinfang.github.io/ReinforcedFDM.html)),( [Video Link](https://www.youtube.com/watch?v=X2o2-SJFv2M)).


