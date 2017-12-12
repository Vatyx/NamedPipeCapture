# NamedPipeCapture
A Windows tool that can be used to stream data from named pipe between two other process to Wireshark through a named pipe

## Installation 

Download the latest version from the [releases](https://github.com/Vatyx/NamedPipeCapture/releases) page.  
Make sure NamedPipeLauncher.exe and NamedPipeCapturex64.dll is in the same directory

## Usage

There are 3 mandatory parameters, 2 optional parameters and 2 commands that can be used.

__Mandatory Parameters__  
* __-\-input__ (shorthand __-i__): Specify the named pipe that will be captured (_Ex_: \myinputpipe)
* __-\-output__ (shorthand __-o__): Specify the named pipe that the data will be sent to (_Ex_: \\\\.\\pipe\\myoutputpipe)
* __-\-processid__ (shorthand __-p__): Specify the process whose functions will be overwritten

__Optional Parameters__  
* __-\-clientport__ (shorthand __-c__): Specify the client-side port. If the process writes something to the pipe, this port will represent the source of the data transfer. If the process reads something from the pipe, this port will be recorded as the destination.
* __-\-serverport__ (shorthand __-s__): Specify the port that will represent any external process reading/writing to the target named pipe.

__Commands__
* __-\-load__ (shorthand __-l__): Load the DLL into the targeted process and open the output named pipe
* __-\-unload__ (shorthand __-u__): Unload the DLL from the application and close the output named pipe

__Example__
```bash
> NamedPipeCapture.exe --input \mynamedpipe --output \\.\pipe\myoutputpipe -processid 21457 -c 50 -s 51 --load
```  
This will target the process with the id `21457`, and the named pipe in the process with the name `mynamedpipe`. The data will be streamed to the named pipe `myoutputpipe` with the client port `50` and the server port `51`

To stop the capture, run the `unload` command with the same process id
```bash
> NamedPipeCapture.exe -processid 21457 --unload
```  

## Streaming Data (using Wireshark)
Once the ``--load`` command has been executed on a process, the data can be seen in real time through Wireshark. Note that data will only begin to be captured once Wireshark has connected to the named pipe specified by the --output parameter.

1. Open up Wireshark and go to Capture -> Interfaces
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/interfaces1.png)
2. Click on Options on the bottom of the Interfaces window
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/manageinterfaces1.png)
3. Click on Manage Interfaces near the top right of the Capture Options window
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/createpipe1.png)
4. Click on New on the left under the pipe tab and put the name of the output named pipe that was specified earlier
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/createpipe2.png)
5. Click Save and Close the window. (The error "The link type of interface [your pipe] was not specified" may pop up which is fine. Just close it.)
6. The newly added named pipe should be listed along with other interfaces in the Capture Options window. Make sure the capture tick box is ticked and then click on start in the bottom right hand corner.
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/manageinterfaces2.png)

Data should now be streaming into Wireshark in real time!
![Tutorial](https://github.com/Vatyx/NamedPipeCapture/blob/master/Images/streaming1.png)

#Make sure Wireshark has stopped capturing data before the DLL is unloaded from the process.
