# Project Reunion - PIP - Test Collaterals - PIPLauncher
Produces a standalone win32 app PIPLauncher.exe which launches the PIP app in various ways. 
The PIP package and the framework package that it depends on must be registered for the current user.
Run PIPLauncher.exe without argument to get help.
The most common usage:
- Launch an app in the PIP by AUMID, e.g.:
PIPLauncher SidecarMainApp_8wekyb3d8bbwe!SidecarMainApp
- Launch an app in the PIP by AUMID _and_ initialize the ExecutablePath string in the AppData of the PIP to the specified path string, e.g.:
PIPLauncher SidecarMainApp_8wekyb3d8bbwe!SidecarMainApp --Init "C:\DirA\App B\bin"
- Invokes the PIP's OOP packaged COM server for LifetimeManager using the default CLSID, e.g.:         
PIPLauncher --Invoke
- Invokes the PIP's OOP packaged COM server for LifetimeManager using the alternate CLSID specified, e.g.:        
PIPLauncher --Invoke {65c056f3-458d-4df1-9a03-3303733f671f}
