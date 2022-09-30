A version of RTKLIB optimized for single and dual frequency low cost GPS receivers, especially u-blox receivers.  It is based on RTKLIB 2.4.3 and is kept reasonably closely synced to that branch.   This software is provided “AS IS” without any warranties of any kind so please be careful, especially if using it in any kind of real-time application.  

Binaries and tutorials for this code, and sample GPS data sets are available at http://rtkexplorer.com/  

The latest version of the user manual is at: https://rtkexplorer.com/pdfs/manual_demo5.pdf

   
WINDOWS: To build and install code for with Windows Embarcadero compiler:

GUIs: 
1) Build executables with app/winapp/rtklib_winapp.groupproj project file 
2) Install executables and DLLs to ../RTKLIB/bin by runnning app/winapp/install_winapp.bat

CUIs:
1) Build executables with app/consapp/rtklib_consapp.groupproj project file 
2) Install executables to ../RTKLIB/bin by runnning app/consapp/install_consapp.bat



LINUX: To build and install code

CUIs:
1) cd app/consapp/<appName>/gcc
2) make

GUIs (Qt based):
1) cd app/qtapp
2) qmake
3) make
4) ./install_qtapp

The linux GUI files have been updated from https://github.com/JensReimann/RTKLIB/tree/rtklib_2.4.3 but at this point may not be fully functional
