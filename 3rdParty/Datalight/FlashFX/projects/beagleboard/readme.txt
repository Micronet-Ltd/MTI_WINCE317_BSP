
		FlashFX Sample Project for the BeagleBoard

	This project is designed to be used with the BeagleBoard with  
	with Microsoft Windows CE 6.0 and the onboard NAND flash.

	Using the RNDIS Driver

	The RNDIS driver implemented in usb8023.sys does not seem to like 
	to co-exist with other network connections.  The following steps
	seem to be what is necessary to make things function.

	1) Disable any other active network connections on the system.
	2) Configure the RNDIS driver to use a static IP address
	3) Configure the BeagleBoard to disable DHCP and also use a static
	   IP address which matches the host with regard to the subnet mask.
	4) At power on, if the BeagleBoard sits at the message:
	
		Rndis:: Get MAC address 200,902F,B5D

 	   For more than about 5 seconds, it is not going to come up.  Press
 	   the reset button (the innermost one) until it comes.  Sometimes
 	   physically removing the power is necessary.  Once it gets past
 	   that message, it always seems to work. 

 	5) Ensure that platform builder is configured for ethernet download
 	   and debugging, not USB.

   