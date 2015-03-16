Do you want to control the volume and mute of one Windows<sup>†</sup> computer on your LAN from another computer?  Try LAN Remote Control.

![http://lanremotecontrol.googlecode.com/svn/images/lanremotecontrol.gif](http://lanremotecontrol.googlecode.com/svn/images/lanremotecontrol.gif)


### Installation ###

  1. There are two components to LAN Remote Control -- _MixerServer.exe_ and _MixerClient.exe_.
  1. Place _MixerServer.exe_ in a directory of your choosing on a Windows computer, the volume/mute of which you would like to remotely control.  When you launch _MixerServer.exe_ a speaker icon will be placed in the system tray.  You can right-click or double-click on this icon to exit the program.
  1. On another Windows computer within your local area network (LAN), place _MixerClient.exe_ in a directory of your choosing.
  1. When launched, MixerClient displays a familiar volume slider and mute button in a compact form, which can be dragged anywhere on the screen.
  1. Right-clicking on the form displays a context menu which allows the user to exit the program, minimize the form to the system tray, or toggle the ability of the form to appear on top of any other window.
  1. When the volume slider is adjusted or the mute button clicked in MixerClient, a message is sent to MixerServer and the main mixer on that computer is affected.

  * MixerServer and MixerClient rely on UDP multicasting to communicate over the LAN.  If you experience any difficulties, make sure that any firewall software or hardware is not interfering with this type of network communication.


### Details ###

Built using Microsoft® Visual Studio® .NET™ 2003 Service Pack 1.  Compilation requires installation of [WTL 8.0](http://sourceforge.net/projects/wtl/).


---


<sup>†</sup>Windows Vista does not support [MME](http://en.wikipedia.org/wiki/MultiMedia_Extensions#Multimedia_Extensions) on which lanremotecontrol relies.