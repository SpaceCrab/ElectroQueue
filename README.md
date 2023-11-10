# ElectroQueue
Distributed EV charging priority system 

## requirements 
### platformio
this project uses platformio to manage dependencies.

https://platformio.org/

To build and upload to the target board either use ``` pio run -t upload``` if you are using Platformio CLI or run it in your IDE of choice with Platformio installed.

### painless mesh 

The mesh network is made using painless mesh.

https://gitlab.com/painlessMesh/painlessMesh

## How to use git 
1. Install git from [here](https://git-scm.com/download)
2. Clone the repository to your machine using ```git clone```  or use the built in git functionality in vs code
3. make a branch for the feature you're working on using ```git checkout -b feature-name``` or ```git branch feature-name```and then ```git checkout feature-name``` this can also be done using the built in git functionality in VsCode
4. write your code and create your files
5. use ```git add filename.c```to add your files to the current commit or ```git add .``` to add all new and changed files
6. use ```git commit```to commit the changed files, this will show a text editor where you write a commit message describing what you've done
7. use ```git push -u origin feature-name``` to push your branch to github
* point 5-7 can all be done inside vscode under the version controll tab in the left sidebar
  
Reference for using git with [VsCode](https://code.visualstudio.com/docs/sourcecontrol/overview)
