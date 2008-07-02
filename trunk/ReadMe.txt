LAN Remote Control (Revision 8) 2 July 2008

http://lanremotecontrol.googlecode.com


Installation:

There are two components to LAN Remote Control -- MixerServer.exe and MixerClient.exe.

Place MixerServer.exe in a directory of your choosing on a Windows computer.  When you launch MixerServer.exe a speaker icon will be placed in the system tray.  You can right-click or double-click on this icon to exit the program.

On another Windows computer within your local area network (LAN), place MixerClient.exe in a directory of your choosing.

When launched, MixerClient displays a familiar volume slider and mute button in a compact form, which can be dragged anywhere on the screen.

Right-clicking on the form displays a context menu which allows the user to exit the program, minimize the form to the system tray, or toggle the ability of the form to appear on top of any other window.

When the volume slider is adjusted or the mute button clicked in MixerClient, a message is sent to MixerServer and the main mixer on that computer is affected.

MixerServer and MixerClient rely on UDP multicasting to communicate over the LAN.  If you experience any difficulties, make sure that any firewall software or hardware is not interfering with this type of network communication.