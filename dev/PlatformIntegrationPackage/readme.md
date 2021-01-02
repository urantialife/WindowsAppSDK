# Project Reunion - PIP - Product code
Folders:
- PIPMainPackage - The PIP's main package, which contains "shim apps" that are minimal exe which delegate most of the work to its corresponding DLL in the framework package.
- PIPFrameworkPackage - The PIP's framework package, to be merged into the ProjectReunion framework package in the future. PIPMainPackage takes a dependence on this. It houses at least one DLL that contains "backend business logic".
- PIPTool - Commandline authoring tool for processing an input PIPAppxManifest.xml to produce the actual AppxManifest.xml to be used by the PIP's main package. It can optionally accept a C header file that contains the default CLSID for DDLM's Lifetime Manager which is to be replaced with a newly generated GUID. The main tasks to do by PIPTool:
-- Filter out Properties elements from Extension elements in the input PIPAppxManifest.xml to produce a "clean" AppxManifest.xml.
-- Replace the default CLSID in PIPAppxManifest.xml with a newly generate GUID while generating AppxManifest.xml. PIPAppxManifest.xml is not altered. If a C header file is specified then the default CLSID will also be replaced consistently with the same new GUID, but the C header filer is altered "in place".

Corresponding test code is under test\PlatformIntegrationPackage.
