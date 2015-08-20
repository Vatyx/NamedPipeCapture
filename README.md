# NamedPipeCapture
A Windows tool that can be used to stream data from named pipe between two other process to Wireshark

## Installation 

Download the latest version from the [releases](https://github.com/Vatyx/NamedPipeCapture/releases) page.  
Make sure NamedPipeLauncher.exe and NamedPipeCapturex64.dll is in the same directory

## Usage

This tool is used through Windows Powershell. There are 3 mandatory parametrs, 2 optional parameters and 2 commands that can be used.

__Mandatory Parameters__  
* __-\-input__ (shorthand __-i__): Specify the named pipe that will be captured (_Ex_: \myinputpipe)
* __-\-output__ (shorthand __-o__): Specify the named pipe that the data will be sent to (_Ex_: \\\\.\\pipe\\myoutputpipe)
* __-\-processid__ (shorthand __-p__): Specify the process whose functions will be overwritten

__Optional Parameters__  
* __-\-clientport__ (shorthand __-c__): Specify the port that will represent the process that was specified. If the process writes something to the pipe, it will look like the data is sent from this port. If the process reads something from the pipe, it will look like the data was recieved at this port.
* __-\-serverport__ (shorthand __-s__): Specify the port that will represent any external process reading/writing to the target named pipe.

__Commands__
* __-\-load__ (shorthand __-l__): Load the DLL into the targeted process and open the output named pipe
* __-\-unload__ (shorthand __-u__): Unload the DLL from the application and close the output named pipe

##### Example
```bash
> NamedPipeCapture.exe --input \mynamedpipe --output \\.\pipe\myoutputpipe -processid 21457 -c 50 -s 51 --load
```  
This will target the process with the id `21457`, and the named pipe in the process with the name `mynamedpipe`. The data will be streamed to the named pipe `myoutputpipe` with the client port `50` and the server port `51`

To stop the capture, run the `unload` command with the same process id
```bash
> NamedPipeCapture.exe -processid 21457 --unload
```  
