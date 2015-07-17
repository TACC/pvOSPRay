pvOSPRay <a href="https://tacc.github.io/pvOSPRay/">https://tacc.github.io/pvOSPRay</a> 
======
<h2>About</h2>
pvOSPRay is a ParaView plugin which creates a custom view using vtkOSPRay for rendering. The current version uses ParaView 4.3.0 on linux using ICC. 

<h2>Using Existing modules on Stampede and Maverick</h2>
<p>modules are provided for running on TACC's Stampede and Maverick clusters.  A system wide release is planned, but for now module files must of custom loaded with the below instructions. 
Note that the $WORK filesystem is shared between Stampede and Maverick (along with other TACC machines) so these steps will work for both machines.</p>
<ul>
<li>module use /work/01336/carson/opt/modulefiles</li>
<li>module load paraview/4.1.0</li>
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
For the latest instructions to build pvOSPRay, please see <a href="http://tacc.github.io/pvOSPRay/getting_pvospray.html">Getting pvOSPRay</a> on the <a href="http://tacc.github.io/pvOSPRay/">pvOSPRay GitHub Pages site</a>.


<h2>Running</h2>
Under Tools->"Manage Plugins" select "Load New..." and navigate to the libOSPRayView.so library. Select OSPRayView and click load selected.  Close the plugins window.
Click the "x" on the top right of the window to close the rendering, and select "OSPRay" to create a pvOSPRay rendering view.


