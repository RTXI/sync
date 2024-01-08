### Sync Module

**Requirements:** None  
**Limitations:** None  

![Sync GUI](sync.png)

<!--start-->
**BEFORE RTXI 3.0.0**
The Sync module lets you synchronize the starting and stopping of a set of loaded modules and the Data Recorder. These modules must be derived from DefaultGUIModel. For each module you want to synchronize, enter its ID number (the number found on the left-hand corner of each widget's titlebar) in the "Module IDs" field as a comma-delimited list. This version of Sync can take a comma with or without an additional space. When the modules are entered, hit "Modify." 

You can also have the module synchronize the starting and stopping of the Data Recorder with your modules. Simply check the "Sync Data Recorder" checkbox.  

To activate synchronized modules, hit unpause this module. To stop them, pause this module. If you use the "Sync Timer" option, you can specify how long the synchronized modules should remain active. 

**AFTER RTXI 3.0.0**
The Sync module lets you synchronize the starting and stopping of a set of loaded modules and the Data Recorder. These modules must be derived from Widgets base class. For each module you want to synchronize, find the module you wish to synchronize in the dropdown field and click the "Sync" button. When the modules are entered, hit "Modify." 

You can also have the module synchronize the starting and stopping of the Data Recorder with your modules. Simply check the "Sync Data Recorder" checkbox. This module does **NOT** automatically record synchronized plugins, so make sure to add the relevant recording plugins to the recorder. 

To activate synchronized modules, hit unpause this module. To stop them, pause this module. If you use the "Sync Timer" option, you can specify how long the synchronized modules should remain active. 


<!--end-->


#### Input Channels
None

#### Output Channels
None

#### Parameters
**RTXI 2.4**
1. Model IDs - modules to synchronize (0-255)
**RTXI 3.0.0**
None

#### States
**RTXI 2.4**
1. Time (s) - elapsed Time (s)
**RTXI 3.0.0**
None


