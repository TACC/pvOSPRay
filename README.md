pvOSPRay <a href="https://tacc.github.io/pvOSPRay/">https://tacc.github.io/pvOSPRay</a>
======
<h2>About</h2>
pvOSPRay is a ParaView plugin which creates a custom view using vtkOSPRay for rendering. Currently tested using ParaView 4.1.0 on linux using ICC.

<h2>Using Existing modules on Stampede and Maverick</h2>
<p>modules are provided for running on TACC's Stampede and Maverick clusters.  A system wide release is planned, but for now module files must of custom loaded with the below instructions. 
Note that the $WORK filesystem is shared between Stampede and Maverick (along with other TACC machines) so these steps will work for both machines.</p>
<ul>
<li>module use /work/01336/carson/opt/modulefiles</li>
<li>module load paraview</li>
<li>module load pvospray</li>
<li>vglrun paraview</li>
<li>the plugin should automatically be loaded. Click the "x" on the top right of the window to close the rendering, and select "OSPRay" to create a pvOSPRay rendering view.</li>
</ul>

<p>You can also use the <a href="http://openswr.org/">OpenSWR</a> library for efficient CPU-based rasterization of the non-ray-traced components. 
To do this, load the OpenSWR module and substitute it for the VirtualGL vglrun command.
</p>
<ul>
<li>module use /work/01336/carson/opt/modulefiles</li>
<li>module load paraview</li>
<li>module load pvospray</li>
<li>module load swr</li>
<li>swr paraview</li>
<li>the plugin should automatically be loaded. Click the "x" on the top right of the window to close the rendering, and select "OSPRay" to create a pvOSPRay rendering view.</li>
</ul>


<h2>Building</h2>
<h3>Prerequisites</h3>
OSPRay requires the same tools as building ParaView, such as CMake.  Additionally, the plugin with require ISPC and OSPRay which are documented below.  It is recommended to use a recent version of the Intel C++ compiler.  
<h3>Building OSPRay</h3>
<ul>
<li>
complete directions for building ospray can be found at: http://ospray.github.io
</li>
<li>
git clone https://github.com/ospray/ospray.git path_to_ospray_dir
</li>
<li>
cd path_to_ospray_dir
</li>
<li>
git checkout 1c917bbf87374cbcb907470a82e99abf25b7ebd2
</li>
<li>
download binaries of ispc 1.8.0 from <a href="https://ispc.github.io/downloads.html">here<a/>
</li>
<li>
CXX="icpc" CC="icc" ccmake .
</li>
<li>
click c for configure, set the ISPC_DIR to the directory with the ispc binary.  click g to generate.
</li>
<li>
make -j4
</li>
</ul>
<h3>Building ParaView</h3>
<ul>
<li>download ParaView 4.1.0 source from the website, or click <a href="http://www.paraview.org/paraview-downloads/download.php?submit=Download&version=v4.1&type=source&os=all&downloadFile=ParaView-v4.1.0-source.tar.gz">here</a></li> to paraview_source_dir. We recommend putting paraview_source_dir in the same directory as the OSPRay source dir.
<li>
cd <paraview_source_dir>/Plugins
</li>
<li>
git clone https://github.com/TACC/pvOSPRay.git pvOSPRay
</li>
<li>
cd paraview_build_dir
</li>
<li>
CXX="icpc" CC="icc" ccmake paraview_source_dir .  
</li>
<li>
configure 
</li>
<li>
enable PARAVIEW_BUILD_PLUGIN_OSPRayView
</li>
<li>
you will need to set the relevant OSPRAY_DIR and OSPRAY_BUILD_DIR to path_to_ospray_dir
</li>
<li>
generate
</li>
<li>
make -j4
</li>
</ul>

<h2>Running</h2>
Under Tools->"Manage Plugins" select "Load New..." and navigate to the libOSPRayView.so library. Select OSPRayView and click load selected.  Close the plugins window.
Click the "x" on the top right of the window to close the rendering, and select "OSPRay" to create a pvOSPRay rendering view.


