#target AfterEffects

/**************************************
Scene : Scene
Resolution : 1920 x 1080
Duration : 10.416667
FPS : 24.000000
Date : 2012-07-17 15:01:12.684000
Exported with io_export_after_effects.py
**************************************/



function compFromBlender(){

var compName = prompt("Blender Comp's Name \nEnter Name of newly created Composition","BlendComp","Composition's Name");
if (compName){
var newComp = app.project.items.addComp(compName, 1920, 1080, 1.000000, 10.416667, 24);
newComp.displayStartTime = 0.083333;


// **************  CAMERA 3D MARKERS  **************


// **************  OBJECTS  **************


var _Plane = newComp.layers.addNull();
_Plane.threeDLayer = true;
_Plane.source.name = "_Plane";
_Plane.property("position").setValue([1012.147365,503.293469,-1.928204],);
_Plane.property("orientation").setValue([-90.000000,-0.000000,0.000000],);
_Plane.property("scale").setValue([459.605455,459.605455,459.605455],);


// **************  LIGHTS  **************


// **************  CAMERAS  **************


var _Camera = newComp.layers.addCamera("_Camera",[0,0]);
_Camera.autoOrient = AutoOrientType.NO_AUTO_ORIENT;
_Camera.property("position").setValue([1850.899563,-102.914200,-1683.722496],);
_Camera.property("orientation").setValue([-20.665957,-24.231479,-8.800195],);
_Camera.property("zoom").setValue(2100.000000,);



}else{alert ("Exit Import Blender animation data \nNo Comp's name has been chosen","EXIT")};}


app.beginUndoGroup("Import Blender animation data");
compFromBlender();
app.endUndoGroup();


