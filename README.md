# Drizzle-plugin
This repository will be used to implement a Drizzle algorithm plugin for Opticks as a part of GSoC 2015.

Progress blog:
http://drizzle-for-opticks-gsoc2015.blogspot.com/

To use plugin:
- Install Opticks: http://opticks.org/confluence/display/opticks/Download
- Download 'Drizzle.dll' from the folder 'plugin' on this repository or build
- Move 'Drizzle.dll' to the PlugIn folder of Opticks (%OPTICKS_DIR%\4.12.0\Plugins\)
- Open Opticks and select the Drizzle plugin from the 'Image Enhancement' drop down menu

To build:
- Install Opticks SDK: http://opticks.org/docs/sdk/4.9.1/
- Go to the folder where Opticks SDK is installed
- Go to application\PlugIns\src\
- Create new directory named 'Drizzle'
- Download and move all files from the 'Code' folder from this repository as well as the Visual Studio project 'Drizzle.vcxproj' and 'Drizzle.vcxproj.user' to this folder.
- Open the solution file 'SamplePlugin.sln' located in the %SDK_FOLDER%\application in Visual Studio 2010
- Add existing project 'Drizzle.vcxproj' to this solution
- Build
