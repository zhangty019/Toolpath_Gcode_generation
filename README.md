# Toolpath Generation and CNC Simulation
Simple example which contains the different toolpath generation, and G-code generation for the 5AXISMAKER machine or UR5e robot


## Installation

Please compile the code with QMake file "TpGene_and_cncSim.pro".

**Platform**: 
	Windows + Visual Studio 2022 + QT-plugin (tested QT version: 5.12.3 + msvc2017_64)

**Install Steps**: 
- **Install Visual Studio Extension plug-in (QT VS Tool)** to open the .pro file and generate the project
- **Set 'shapeLab' as the start up project**
- **Enable OpenMP to get best performace** at: ShapeLab Project Property -> 'Configuration Proerties' -> c/c++ -> Language -> Open MP Support -> Select 'Yes (/openmp)'
- **Open Console** at: ShapeLab Project Property -> 'Configuration Proerties' -> Linker -> System -> Select 'Console (/SUBSYSTEM:CONSOLE)' in 'SubSystem'

## Usage

**Step 0: input layer files (.obj) .**
Prerequisite: Remesh the surface layer ( python + meshlab 2022.02 )
- source directory: ../DataSet/source_layer/input; 
- double click "remesh.py" to do remesh.
- So, the files of the layers (.obj) for the model will be saved in the following folder
'**../DataSet/source_layer/output**'

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

Prerequisite: 
- Copy the "CURVED_LAYER" and "TOOL_PATH" in "../DataSet" and save them into "../DataSet/FABRICATION/_modelName_/" 

Note: "_modelName_" can be decided by user, e.g. "Dome".

- Remesh the surface layer ( python + meshlab 2022.02 ) to make the layer sparse and speed up the calculation of layer height for extrusion volume calculation
-- source directory: ../DataSet/FABRICATION/_modelName_; 
-- double click "remesh.py" to do remesh.
-- So, the files of the layers (.obj) for the model will be saved in the following folder
'../DataSet/FABRICATION/_modelName_/layer_Simplified'

- In the '**Model**' box, input the folder name of the needed model, which is stored at :
 '**../DataSet/FABRICATION/_modelName_/layer_Simplified**' and '**../DataSet/FABRICATION/_modelName_/waypoint**'
 
- Click the '**1. Read data**'‘,
--the programme will automatically load the  Waypoints and Slices (Obj.) files.
--Input the range of needed layers in 'From' and To'
--Input tool length in 'toolLns'

- Click the '**2. Calculate Extrusion Volume**',
--The volume of the material will be calculated base on the waypoint width, distance and height.

- Click the '**3. Singularity Optimization**',
--It will use the position and normal of waypoints to calculate the movements of the 5axis to reduce the nagetive effect of Singularity.

- Click the'**4. G Code Writing** '
-- The G code will be stored at folder 
'**../DataSet/G_CODE**'

- Click the **Simulate** 
--The **progress bar** shows the percentage of completed print volume, and clicking the checkbox '**stop**' will stop the simulation from running.

**Notes**: This project also support the waypoint generation for the UR robot

- Prerequisite: 

-- open the ../DataSet/FABRICATION/robotWpGeneration" and duble click "copy_and_remesh.py", it will copy the "CURVED_LAYER" and "TOOL_PATH" in "../DataSet" and save them into "../DataSet/FABRICATION/" 

- Click the '**1. Ur5e robot Gcode Generation**'‘,

-- the programme will automatically load the  Waypoints(*.txt) and Slices (*.obj) files.

-- output the data for ur5e robot into "../DataSet/FABRICATION/toolpath_robot_ur/"

## Curved Layer Generation Algorithm

[Tianyu Zhang](https://www.linkedin.com/in/tianyu-zhang-49b8231b5/), Guoxin Fang, Yuming Huang, Neelotpal Dutta, Sylvain Lefebvre, Zekai Murat Kilic, and [Charlie C.L. Wang](https://mewangcl.github.io/), [*ACM Transactions on Graphics (SIGGRAPH Asia 2022)*, vol.41, no.6, article no.277 (15 pages), December 2022](https://dl.acm.org/doi/10.1145/3550454.3555516)
