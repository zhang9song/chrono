# =============================================================================
# PROJECT CHRONO - http://projectchrono.org
#
# Copyright (c) 2014 projectchrono.org
# All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file at the top level of the distribution and at
# http://projectchrono.org/license-chrono.txt.
#
# =============================================================================


import pychrono as chrono
import pychrono.irrlicht as chronoirr

import errno
import os
import importlib.util

enable_povray_export = False

postprocess_available = importlib.util.find_spec('pychrono.postprocess')
if postprocess_available and enable_povray_export:
    import pychrono.postprocess as postprocess


# The path to the Chrono data directory containing various assets (meshes, textures, data files)
# is automatically set, relative to the default location of this demo.
# If running from a different directory, you must change the path to the data directory with: 
#chrono.SetChronoDataPath('path/to/data')


# ---------------------------------------------------------------------
#
#  Create the simulation system.
#  (Do not create parts and constraints programmatically here, we will
#  load a mechanism from file)

sys = chrono.ChSystemNSC()
sys.SetCollisionSystemType(chrono.ChCollisionSystem.Type_BULLET)

# Set the collision margins. This is expecially important for very large or
# very small objects (as in this example)! Do this before creating shapes.
chrono.ChCollisionModel.SetDefaultSuggestedEnvelope(0.001);
chrono.ChCollisionModel.SetDefaultSuggestedMargin(0.001);


# ---------------------------------------------------------------------
#
#  Load the file generated by the SolidWorks CAD plugin and add it to the system
#

print ("Loading Chrono scene...");

exported_items = chrono.ImportSolidWorksSystem(chrono.GetChronoDataFile('solidworks/swiss_escapement.py'))

print ("...done!");

# Print exported items
for item in exported_items:
    print (item.GetName())

# Add items to the physical system
for item in exported_items:
    sys.Add(item)

# ---------------------------------------------------------------------
#
#  Setup POV-Ray postprocessing if required
#
if postprocess_available and enable_povray_export:

    # ---------------------------------------------------------------------
    #
    #  Render a short animation by generating scripts
    #  to be used with POV-Ray
    #

    pov_exporter = postprocess.ChPovRay(sys)

    # Sets some file names for in-out processes.
    pov_exporter.SetTemplateFile(chrono.GetChronoDataFile('POVRay_chrono_template.pov'))
    pov_exporter.SetOutputScriptFile("rendering_frames.pov")

    try:
        os.mkdir("output")
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            print("Error creating output directory " )

    try:
        os.mkdir("anim")
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            print("Error creating anim directory " )

    pov_exporter.SetOutputDataFilebase("my_state")
    pov_exporter.SetPictureFilebase("anim/picture")

    # Sets the viewpoint, aimed point, lens angle
    pov_exporter.SetCamera(chrono.ChVector3d(0.2,0.3,0.5), chrono.ChVector3d(0,0,0), 35)

    # Sets the default ambient light and default light lamp
    pov_exporter.SetAmbientLight(chrono.ChColor(1,1,0.9))
    pov_exporter.SetLight(chrono.ChVector3d(-2,2,-1), chrono.ChColor(0.9,0.9,1.0), True)

    # Sets other settings
    pov_exporter.SetPictureSize(640,480)
    pov_exporter.SetAmbientLight(chrono.ChColor(0.8,0.8,0.8))

    # If wanted, turn on the rendering of COGs, reference frames, contacts:
    #pov_exporter.SetShowCOGs  (1, 0.05)
    #pov_exporter.SetShowFrames(1, 0.02)
    #pov_exporter.SetShowLinks(1, 0.03)
    #pov_exporter.SetShowContacts(1,
    #                            postprocess.ChPovRay.SYMBOL_VECTOR_SCALELENGTH,
    #                            0.01,   # scale
    #                            0.0007, # width
    #                            0.1,    # max size
    #                            1,0,0.5 ) # colormap on, blue at 0, red at 0.5

    # Add additional POV objects/lights/materials in the following way, entering
    # an optional text using the POV scene description laguage. This will be
    # appended to the generated .pov file.
    # For multi-line strings, use the python ''' easy string delimiter.
    pov_exporter.SetCustomPOVcommandsScript(
    '''
    light_source{ <1,3,1.5> color rgb<0.9,0.9,0.8> }
    ''')

    # Tell which physical items you want to render
    pov_exporter.AddAll()


    # 1) Create the two .pov and .ini files for POV-Ray (this must be done
    #    only once at the beginning of the simulation).
    pov_exporter.ExportScript()

# ---------------------------------------------------------------------
#
#  Create an Irrlicht application to visualize the system
#

vis = chronoirr.ChVisualSystemIrrlicht()
vis.AttachSystem(sys)
vis.SetWindowSize(1024,768)
vis.SetWindowTitle('Test: using data exported by Chrono::Solidworks')
vis.Initialize()
vis.AddLogo(chrono.GetChronoDataFile('logo_pychrono_alpha.png'))
vis.AddSkyBox()
vis.AddCamera(chrono.ChVector3d(0.3,0.3,0.4))
vis.AddTypicalLights()


# ---------------------------------------------------------------------
#
#  Run the simulation
#


 # Configure the solver
sys.SetSolverType(chrono.ChSolver.Type_BARZILAIBORWEIN)
sys.GetSolver().AsIterative().SetMaxIterations(40)
sys.SetMaxPenetrationRecoverySpeed(0.002)
sys.SetGravitationalAcceleration(chrono.ChVector3d(0,-9.8,0))


while vis.Run():
    vis.BeginScene()
    vis.Render()
    vis.EndScene()
    sys.DoStepDynamics(0.002)

    if postprocess_available and enable_povray_export:
        # 2) Create the incremental nnnn.dat and nnnn.pov files that will be load
        #    by the pov .ini script in POV-Ray (do this at each simulation timestep)
        pov_exporter.ExportData()


