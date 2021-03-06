PENTrack
========

PENTrack - a simulation tool for ultra-cold neutrons, protons and electrons

If you just want to do simulations, you should check out the stable releases, which have been tested. If you want the latest and greatest features and probably some bugs, or even want to contribute, you should check out the latest revision from master branch.


External libraries
-----------

### CGAL

The [Computational Geometry Algorithms Library](http://www.cgal.org/) is used to detect collisions of particle tracks with the experiment geometry defined by triangle meshes using AABB trees.
Some Linux distributions (e.g. Ubuntu, Debian) include the libcgal-dev package, for all others, it has to be downloaded and installed manually from the website. In the latter case you may have to adjust the cmake search path by setting the CGAL_DIR variable by calling cmake with `-DCGAL_DIR=/path/to/CGAL`.

CGAL v4.1 - v4.8 have been tested.

### Boost

The [Boost C++ libraries](https://www.boost.org/) are a prerequisite for the CGAL library. Additionally, the simulation uses a 64bit Mersenne Twister pseudo-random number generator and the [odeint integrator](http://headmyshoulder.github.io/odeint-v2/) included in Boost 1.53.0 and newer. Boost is included in most Linux OSs; should you need to download and compile it manually, you may have to adjust the cmake search path by setting the BOOST_ROOT variable by calling cmake with `-DBOOST_ROOT=/path/to/boost`.

Boost 1.53.0 - 1.59.0 have been tested. 1.56.0 and newer require a C++11 capable compiler (i.e. GCC 4.8+).

### muparser

[muparser](http://muparser.beltoforion.de/) is a fast formula parser and is used to interpret energy distributions etc. given by the user in particle.in.

It is included in the repository.

### ALGLIB

[ALGLIB](http://www.alglib.net) is used to do 1D and 2D interpolation for field calculations.
It also provides numerical optimization and integration routines required for the MicroRoughness model for UCN interaction with matter.

It is included in the repository.

### libtricubic

[Lekien and Marsden] (http://dx.doi.org/10.1002/nme.1296) developed a tricubic interpolation method in three dimensions. It is included in the repository.


Run the simulation
------------------

Type `cmake .` to create a Makefile, execute `make` to compile the code, then run the executable. Some information will be shown during runtime. Log files (start- and end-values, tracks and snapshots of the particles) will be written to the /out/ directory, depending on the options chosen in the *.in files.
Three command line parameters can be passed to the executable: a job number (default: 0) which is appended to all log file names, a path from where the *.in files should be read (default: in/) and a path to which the out file will be written (default: out/).


Defining your experiment
------------------------

### Geometry

Geometry can be imported using [binary STL files] (http://en.wikipedia.org/wiki/STL_%28file_format%29).
STL files use a list of triangles to describe 3D-surfaces. They can be created with most CAD software, e.g. Solidworks via "File - Save As..." using the STL file type. Solidworks makes sure that the surface normals always point outside a solid body. This is used to check if points lie inside a solid object.
Note the "Options..." button in the Save dialog, there you can change format (only binary supported), resolution, coordinate system etc.

PENTrack expects the STL files to be in unit Meters.

Gravity acts in negative z-direction, so choose your coordinate system accordingly.

Do not choose a too high resolution. Usually, setting the deviation tolerance to a value small enough to represent every detail in your geometry and leaving angle tolerance at the most coarse setting is good enough. Low angle tolerance quickly increases triangle count and degeneracy.

If you want to export several parts of a Solidworks assembly you can do the following:

1. Select the part(s) to be exported and rightclick.
2. Select "Invert selection" and then use "Suppress" on all other parts.
3. Now you can save that single part as STL (make sure the option "Do not translate STL output data to positive space" is checked and to use the same coordinate system for every part or else the different parts will not fit together).
4. You can check the positioning of the parts with e.g. [MeshLab](http://meshlab.sourceforge.net/), SolidView, Minimagics, Solidworks...

### Fields

Magnetic and electric fields (rotationally symmetric 2D and 3D) can be included from text-based field maps
(right now, only "[Vectorfields OPERA](http://www.operafea.com)" maps in cgs units are supported).
You can also define analytic fields from straight, finite conductors.

### Particle sources

Particle sources can be defined using STL files or manual parameter ranges. Particle spectra and velocity distributions can also be conviniently defined in the particle.in file.


Physics
-------

All particles use the same relativistic equation of motion, including gravity, Lorentz force and magnetic force on their magnetic moment. UCN interaction with matter is described with the Fermi potential formalism and either the [Lambert model](https://en.wikipedia.org/wiki/Lambert%27s_cosine_law) or the MicroRoughness model (see see [Z. Physik 254, 169--188 (1972)](http://link.springer.com/article/10.1007%2FBF01380066) and [Eur. Phys. J. A 44, 23-29 (2010)](http://ucn.web.psi.ch/papers/EPJA_44_2010_23.pdf)) for diffuse reflection and includes spin flips on wall bounce. Protons and electrons do not have any interaction so far, they are just stopped when hitting a wall. Spin tracking by bruteforce integration of the Bloch equation is also included.



Writing your own simulation
---------------------------

You can modify the simulation on four different levels:

1. Modify the /in/*.in configuration files and use the implemented particles, sources, fields, etc. (more info below and in the corresponding *.in files)
2. Modify the code in main.c and combine TParticle-, TGeometry-, TField-, TSource-classes etc. into your own simulation (you can generate a Doxygen documentation by typing `doxygen doxygen.config`)
3. Implement your own particles, sources or fields by inheriting from the corresponding base classes and fill the virtual routines with the corresponding physics
4. Make low level changes to the existing classes


Output
-------

Output files are separated by particle type, (e.g. electron, neutron and proton) and type of output (endlog, tracklog, ...). Output files are only created if particles of the specific type are simulated and can also be completely disabled for each particle type individually by adding corresponding variables in 'particle.in'. All output files are tables with space separated columns; the first line contains the column name.

Types of output: endlog, tracklog, hitlog, snapshotlog, spinlog.

### Endlog

The endlog keeps track of the starting and end parameters of the particles simulated. In the endlog, you get the following parameters:

- jobnumber: corresponds to job number of the PENTrack run (command line parameter of PENTrack). Only used when running multiple instances of PENTrack simultaneously.
- particle: number which corresponds to particle being simulated 
- tstart: time at which particle is created in the simulation [s]
- xstart, ystart, zstart: coordinate from which the particle starts [m]
- vxstart, vystart,vzstart: velocity with which the particle starts [m/s]
- polstart: initial polarization of the particle (-1,1)
- Hstart: initial total energy of particle [eV]
- Estart: initial kinetic energy of the particle [eV]
- Bstart: magnetic field at starting point [T]
- Ustart: electric potential at starting point [V]
- solidstart: number of geometry part the particle started in, see geometry.in
- tend: time at which particle simulation is stopped [s]
- xend, yend, zend: coordinate at which the particle ends [m]
- vxend, vyend, vzend: velocity with which the particle ends [m/s]
- polend: final polarization of the particle (-1,1)
- Hend: final total energy of particle [eV]
- Eend: final kinetic energy of the particle [eV]
- Bend: magnetic field at stopping point [T]
- Uend: electric potential at stopping point [V]
- solidend: number of geometry part the particle stopped in, defined in geometry.in
- stopID: code which identifies why the particle was stopped (defined in globals.h)
  - 0: not categorized
  - -1: did not finish (reached max. simulation time)
  - -2: hit outer poundaries
  - -3: produced error during trajectory integration
  - -4: decayed
  - -5: found no initial position
  - -6: produced error during geometry collision detection
  - -7: produced error during tracking of crossed material boundaries
  - 1: absorbed in bulk material (see solidend)
  - 2: absorbed on total reflection on surface (see solidend)
- NSpinflip: number of spin flips that the particle underwent during simulation
- spinflipprob: probability that the particle has undergone a spinflip, calculated by bruteforce integration of the Bloch equation
- Nhit: number of times particle hit a geometry surface (e.g. wall of guidetube)
- Nstep: number of steps that it took to simulate particle
- trajlength: the total length of the particle trajectory from creation to finish [m]
- Hmax: the maximum total energy that the particle had during trajectory [eV]

### Snapshotlog

Switching on snapshotlog in "particle.in" will output the particle parameters at additional 'snapshot times' (also defined in "particle.in") in the snapshotlog. It contains the same data fields as the endlog.

### Tracklog

The tracklog contains the complete trajectory of the particle, with following parameters:

- jobnumber: corresponds to job number of the PENTrack run (command line parameter of PENTrack). Only used when running multiple instances of PENTrack simultaneously.
- particle: number which corresponds to particle being simulated 
- polarization: the polarization of the particle (-1,1)
- t: time [s]
- x,y,z coordinates [m]
- vx,vy,vz: velocity [m/s]
- H: total energy [eV]
- E: kinetic energy [eV]
- Bx: x-component of Magnetic Field at coordinates [T]
- dBxdx, dBxdy, dBxdz: gradient of x-component of Magnetic field at coordinates [T/m]
- By : y-component of magnetic field at coordinates[T]
- dBydx, dBydy, dBydz : gradient of y-component of Magnetic field at coordinates [T/m]
- Bz: y-component of magnetic field at coordinates[T]
- dBzdx, dBzdy, dBzdz: gradient of z-component of Magnetic field at coordinates [T/m]
- Ex, Ey, Ez: X, Y, and Z component of electric field at coordinates [V/m]
- V: electric potential at coordinates [V]

### Hitlog

This log contains all the points at which particle hits a surface of the experiment geometry. This includes both reflections and transmissions.

In the hitlog, you get the following parameters:

- jobnumber: number which corresponds to job number of the PENTrack run (command line parameter of PENTrack). Only used when running multiple instances of PENTrack simultaneously.
- particle: number which corresponds to particle being simulated
- t: time at which particle hit the surface [s]
- x, y, z: coordinate of the hit [m]
- v1x, v1y,v1z: velocity of the particle before it interacts with the new geometry/surface [m/s]
- pol1: polarization of particle before it interacts with the new geometry/surface (-1,1)
- v2x, v2y,v2z: velocity of the particle after it interacted with the new geometry/surface [m/s]
- pol2: polarization of particle after it interacted with the new geometry/surface (-1,1)
- nx,ny,nz: normal to the surface of the geometry that the particle hits
- solid1: ID number of the geometry that the particle starts in
- solid2: ID number of the geometry that the particle hits

### Spinlog

If bruteforce integration of particle spin precession is active, it will be logged into the spinlog. In the spinlog, you get the following parameters:

- t: time [s]
- Polar: projection of Bloch spin vector onto magnetic field direction vector (Bloch vector has dimensionless length 0.5, so this quantity is also dimensionless)
- logPolar: logarithm of above value [dimensionless]
- Ix, Iy, Iz: x, y and z components of the Bloch vector (times 2) [dimensionless]
- Bx, By, Bz: x, y and z components of magnetic field direction vector [T]
- wL: Larmor precession frequency (Hz)
- x1, y1, z1: Position of the neutron at the starting step of TBFIntegrator integration
- v1x, v1y. v1z: Velocity of the neutron at the starting step of TBFIntegrator integration
- E1x, E1y, E1z: Electric field at the neutron's position at the starting step of TBFIntegrator integration
- dE1xdx, dE1xdy, dE1xdz: gradient of the x-component of the electric field at the neutron's position at the starting step of TBFIntegrator integration
- dE1ydx, dE1ydy, dE1ydz: gradient of the y-component of the electric field at the neutron's position at the starting step of TBFIntegrator integration
- dE1zdx, dE1zdy, dE1zdz: gradient of the z-component of the electric field at the neutron's position at the starting step of TBFIntegrator integration
- x2, y2, z2: Position of the neutron at the ending step of TBFInterator integration
- v2x, v2y, v2z: Velocity of the neutron at the ending step of TBFIntegrator integration
- E2x, E2y, E2z: Electric field at the neutron's position at the ending step of TBFIntegrator integration
- dE2xdx, dE2xdy, dE2xdz: gradient of the x-component of the electric field at the neutron's position at the ending step of TBFIntegrator integration
- dE2ydx, dE2ydy, dE2ydz: gradient of the y-component of the electric field at the neutron's position at the ending step of TBFIntegrator integration
- dE2zdx, dE2zdy, dE2zdz: gradient of the z-component of the electric field at the neutron's position at the ending step of TBFIntegrator integration

### Writing Output files to ROOT readable files

merge_all.c: A [ROOT](http://root.cern.ch) script that writes all out-files (or those, whose filename matches some pattern) into a single ROOT file containing trees for each log- and particle-type.

Helper Scripts 
--------------

### preRunCheck.sh

This script performs some preliminary checks before launching a large batch PENTrack job. The checks performed are:

1. Check that the STL files referenced in geometry.in exist (i.e. there is no mistake in the path specified in the [GEOMETRY] section of the file.
2. Checks that all the files in the STL folder are referenced in the [GEOMETRY] section of geometry.in (currently the script looks for a folder name STL this could be made general later)
3. Checks if the number of jobs running would exceed 2880 when the batch file being generated is submitted (2880 is the maximum number of jobs allowed to be running by a single user on [Westgrid's](https://www.westgrid.ca/) [Jasper](https://www.westgrid.ca/support/systems/Jasper/) cluster). The number 2880 could be changed directly in the code depending on a different user requirement.
4. Check if the job numbers being specified in the batch file being generated would overwrite the jobs that already exist in the output directory. A warning is given if there are cases of overwrite and the job numbers affected by the overwrite are given. 

The script also generates a batch.pbs that can be submitted to a TORQUE queueing system. The batch script can be customized by entering the following command line input in this order: 

1. minJobNum - the lower bound of the job array id specified in the batch file (default is 1) 
2. maxJobNum - the upper bound of the job array id specified in the batch file (default is 10, so if minJobNum=1 and maxJobNum=10, submitting the batch file would result in 10 PENTrack jobs being run the job numbers ranging from 1-10)
3. jobName - the name of the job that will displayed when you check the status of the jobs using the `qstat` command (default is SampleRun)
4. inDirName - the relative path to the directory containing PENTrack's input files (the geometry.in file in this directory will be checked, default is in/)
5. outDirName - the relative path to the directory where the job output will be written (the cases for overwriting previous jobs will be checked in this directory, default is out/)
6. pbsfileName - the name of the batch file being generated (default name is batch.pbs)
7. wallTime - the walltime requested from the queueing system, it must be entered in the following format: 1d4h30m which means the time requested is: 1 day + 4 hours + 30 minutes (you can only specify days or hours or minutes but they order must remain: day, hours, minutes). You can also specify 120m and the batch file will correctly set the time to 2 hours. A warning will be given 

### postRunCheck.sh

This script performs some basic error checks after a batch run of PENTrack has finished execution. The two required inputs are: 

1. expectedNumJobs - the number of jobs that were part of the batch run which is to be checked
2. outDirName - the relative path to the output directory where the output from the batch run was written 

The checks performed by the script are: 

1. Lists the number of each type of log file present in the outDirName directory so that you can verify the correct number of log files are produced 
2. Lists the number of empty and non-empty error files and the job numbers for the non-empty error files so you can identify jobs that encountered problems. Currently, the error files are expected to be named error.txt-jobNum where jobNum is an integer, ex: error.txt-1, error.txt-2 etc...
3. Lists the number of output files that had an exit code so you can identify the number of jobs that ran to completion, and the number of jobs that had an exit code of 0 so you can verify the number of jobs that finished normally. A list of the job numbers that did not finish i.e. did not have an exit code and a list of the job numbers that did not have an exit code of 0 is also provided. 
4. Creates a directory named ErrorJobs in the outDirName directory and moves all the files related to a job number with a non-empty error file, file without an exit code and files without an exit code of 0 to the ErrorJobs directory. 
5. Generates a table listing the average, minimum and maximum values of the following simulation parameters: Simulation time, Particle Stop IDS (neutron absorbed on a surface, Neutrons absorbed in a material, Neutrons not categorized etc...). The job numbers corresponding to the jobs with the maximum and minimum values for each parameter are given. This table is useful in making sure there are not too many neutrons encountering geometry errors or hitting the outer boundaries or identifying a job number that had particularly large numbers of neutrons encounter an error etc. 

### genGeomList.sh 

This script generates a list of STL files in the format required by PENTrack, priority and material type defined. This is useful when running a simulation with a large number of STL files required. However, to function the STL files must be named according to a specific naming convention for regular files the convention is: 

 partName_mater_CuBe_prior_1.stl

where CuBe specifies the mater(ial) and 1 specifies the PENTrack priority to be assigned to the part. If the part corresponds to a source volume the naming convention is :

 partName_prior_source.stl

If the file doesn't contain the keyword mater then the STL file will still be displayed in the output but the material will be given as MISSING_MATERIAL. Also, if the material is specified as NotUsed (case insensitive) then file will not be listed in the output. 

The output from the script is directed to standard out is meant to be copied into the geometry.in. The file also takes in two optional arguments: 

1. stlDirName - the relative path to the directory containing the STL files that are to be processed
2. specialPathToSTL - specify this if you wish to append a prefix to the relative path of the STL files. This is useful if you are running the script in a location that has a different relative path to the STL directory in comparison to the location of the PENTrack executable. 


