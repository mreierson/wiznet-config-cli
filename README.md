## wiznet-config-cli

Command-line utility for configuring WizNet serial-to-ethernet modules

`wiznet-config-cli:`  
`  -d [ --discovery ]` Network discovery of WIZnet devices    
`  --ipconfig arg `    MAC address of device to configure  
`  --static-ip arg `   Assign IP address for static configuration  
`  --static-netmask arg (=255.255.255.0) ` Assign netmask for static configuration  
`  --static-gateway arg ` Assign gateway for static configuration  
`  --dhcp               ` Assign DHCP configuration  
`  --port arg           ` Assign WIZnet server port  
`  -b [ --baud ] arg` Assign baud rate (4800,9600,19200,38400,57600,115200,230400)  
`  --mode-client ` Set the operation mode to CLIENT  
`  --mode-server ` Set the operation mode to SERVER  
`  --mode-mixed  ` Set the operation mode to MIXED  
`  --inactivity arg `  Set the inactivity timeout in seconds (0-65535, 0 to disable)  
`  --reset ` Reset the WIZNet device  
`  -h [ --help ] `  Display this help message  
