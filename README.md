# JEPO-1.1
Geant4 Simulation of Polarimeter setup of COSY Storage Ring

FORK rules!
Fine .... relax!!!
You got that????

##################################################
#   Instructions                                      #
##################################################

1. create folder "build" inside the source root folder
2. run `cmake ..` from build

(or)

1. run build.sh or build_mac.sh

custom experiments can be run by using/modifying run.sh

##################################################
#   History                                      #
##################################################

= 6. Dec. 2017. by Hoyong Jeong = 
  - Teflon, tedlar, kapton have been added.
    These surround the crystals.
    Thicknesses are 50 um, 50 um and 25 um respectively.
    Whereas front direction has double thickness for each of layers.

= 11. Apr. 2020. by Rahul Shankar = 
  - Triangular Bar Trackers added (Front stack is fixed and rear stack is movable in the Y direction).
  - Activated option to turn on/off dE Scintillators.
  
= 28. Apr. 2020. by Rahul Shankar =
  - Added customizability to tracker-bar setup via config file.
  - Added feature to sequentially name the tracker bars drawn.
  
= 28. Jan. 2021. by Rahul Shankar =
  - Added ability to switch between parallel and perpendicular configurations of tracker bars
    in config file.
  - Disabled switches of fixed and moving columns.
