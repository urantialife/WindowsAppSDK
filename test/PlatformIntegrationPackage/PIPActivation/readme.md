# Project Reunion - PIP - Test Collaterals – PIPActivation
TAEF functional test that exercises various PIP activation scenarios by invoking PIPLauncher.exe. A few basic scenarios:
-	Basic launch – verify that PIPLauncher.exe itself launches fine and can show the help page.
-	Activation by AUMID – verify that:
-- the PIP's Centennail app can be activated by specifying the AUMID
-- the PIP's Centennail correctly initializes at first launch, especially populating its AppData with contents from its PIPAppxmanifest.xml
-- the PIP app can accept an optional path string to be captured into its AppData as the ExecutablePath field. This simulates a win32 app telling its associated PIP package about the win32 app’s executable path.
-	Activation of the Lifetime Manager server for Dynamic Dependence – This is and older design in which the DDLM resides in the PIP package (versus a 2nd global Reunion main package mainly for DDLM). PIPLauncher.exe simulates a win32 app trying involving the DDLM’s API over OOP packaged COM.

	
The PIP's Centennial app can also be activated via an Execution Aliase. This seems to be useful for troubleshoot, but this scenario currently does not have automated test coverage.