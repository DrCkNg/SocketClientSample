Socket Client Sample
====================

Socket Client Sample is a simple Socket Client application which send trigger commands to a dedicated Port of a Server. Command format are customisable in setting file, with CR(13) LF(10) as string terminator. Each client waits for its reply from the Server and increase Good counter if the reply data is not [EMPTY]/ERROR/Disconnected, Bad Counter will be increased otherwise.

Settings:
---------
Customization settings can be located in the running folder config.ini file's [Device_x] section, which x is client ID start from 1. Number of Device Control is the total number of client devices. 

Device Label:  Name of the device to show on dialog.
               Default(DeviceX)
Enable:  Enable individual device connection when click Connect button if Set to Yes. 
         Default(Yes)
IP Address: IP Address of the server to be connected.
            Default(127.0.0.1) 
Port: Port of the IP address of the server to be connected.
            Default(10038) 
Trigger Command: Trigger Command to be sent to the server
                 Default(SxTRIG)

Operation:
---------
Connect Devices:    Connect all enabled devices in the list to server IP.
Disconnect Devices: Disconnect all devices in the list.
Reset: 		    Reset counters and status on display.

-Device Settings section (Only available when devices disconnected)
Modify: Enter modify mode. 
Back:   Back to life mode with Connect Devices button available.
Apply:  Apply changes to device item after modification.
Load:   Load current file settings.
Save:   Save current device settings into file.

- Testing section (only available when devices connected)
Single Trigger:  Send selected device Trigger command once, and report result.
Loop Single:     Send selected device Trigger command, and report result.                       Repeat the process until user click Stop Loop.
Loop All:        Send all devices Trigger command, and report result.                           Repeat the process until user click Stop Loop.

Exit:	Exit application




Copyright (c) [2018] [DrCkNg@mail.com] for JKBS project.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
