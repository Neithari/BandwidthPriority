# BandwidthPriority
This software is will give a selected application the least latency possible while delaying others.
Thanks to [WinDivert](http://reqrypt.org/windivert.html) it will capture all your packets, prioritize the chosen app
and delay the other packets as needed.

The programm must be run in Administrator mode for WinDivert to work.
The manifest file does need a "requireAdministrator" level flag to debug it with Visual Studio.
