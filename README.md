# Gremlins

![Gizmo](https://github.com/ch3rn0byl/Gremlins/blob/master/Images/Gremlins-Gizmo-square.png)

This application and driver was made to aid for Research and Development purposes in regards to monitoring and fuzzing. The logic for fuzzing is not yet implemented. What this does _not_ do is bypass any security mitigations for malicious behavior. All work that has been put into it as of now has been developed targetting Windows Enterprise 21h2 x64. 
## Requirements for Building Yourself
1. Visual Studio 
2. Windows Driver Kit

This was built using Visual Studio 2022, but could probably be used with older verions.

## Hooking
Gremlins will hook `nt!NtDeviceIoControlFile` out of the box, but could also do others if implemented. Gizmo will resolve all syscalls for any target function you are interested in hooking. The hook is as follows:
```
mov rax, hooked_address
jmp rax
```

The hooking implementation, itself, is bare minimum meaning it will only print the arguments and buffer if there is any. At this point, the function is yours to play with. 

## How does this work?
#### Usermode Application: Gizmo
Gizmo is a usermode application will resolve the syscall for a function using LoadLibrary and then scan that region of memory until it hits the syscall signature. Once the syscall signature is found, it will extract that number and return it. The syscall value is of type UINT16. Gizmo will then forward that information to Gremlins. 

#### Kernel Driver: Gremlins
Gremlins is a kernel driver that will do all the processing. Gremlins will do the following:
1. Check if it's initialized or not.
2. Initialize itself. What this will do is gather all the information needed for the other functions to work correctly. 
3. Check if a given syscall is hooked.
4. If the syscall is not hooked, it will hook it. 
5. If a syscall is hooked, it will restore it. 
Gremlins will first resolve `nt!KiSystemServiceUser` by querying the LSTAR register. Once that is found, it will search KiSystemServiceUser for `nt!KeServiceDescriptorTable`. From there, it has the values for the ServiceTableBase and the amount of services available. 

Gremlins will process the data from Gizmo and then retrieve the function that correlates to that specific syscall index. Gremlins will then convert the virtual address to physical address that way we can double map the physical address into userspace as Read/Write. The patch is applied and the secondary address is unmapped. The Nt function is now hooked, all without the need to modify CR0.[WP].

It uses a LIST_ENTRY to hold the hooked functions data. Doing it this way makes it easier for Gremlins to keep track of whats hooked and what is not hooked. The only limitation of the number of hooks you can use is limited to what you're willing to do and whatever the amount is, if any, of the amount of items LIST_ENTRY's can hold. 

## How to use?
Since this is purely for Research and Development purposes, I make use of debugging print statements to see the output of the system, therefore, being connected to your kernel debugger instance is a must; however, I don't enforce it in Gizmo. It will just pause but you will have no idea what is happening. You will be blind.

A service will need to be created and started with the Service Controller Manager:
```
sc create gremlins binPath= <path to driver>/gremlins.sys type= kernel
sc start gremlins
```
If you want it to automatically at boot, you can use:
```
sc create gremlins binPath= <path to driver>/gremlins.sys type= kernel start= auto
```

As for implementing additional syscalls, the following steps would need to take place to do so:
1. Implement your hooks and place the header(s) and cpp file(s) inside the "Hooks" filter for tidiness.
2. Add the syscall index number ![here](https://github.com/ch3rn0byl/Gremlins/blob/775a032539539ff5839fa1451d730a15afcf9637/Gremlins/typesndefs.h#L33).
3. Add a case statement for your hook ![here](https://github.com/ch3rn0byl/Gremlins/blob/775a032539539ff5839fa1451d730a15afcf9637/Gremlins/dispatchfunctions.cpp#L114). The `NtDeviceIoControlFile` example is an example of implementing it. 
4. Draw the rest of the owl. 

Gremlins logs errors, warnings, tracing, and info messages. The error messages will always be viewed within the debugger. You will need to enable the proper bit mask to view the other messages. If you are interested in viewing warning messages, apply the mask:
```
ed nt!Kd_IHVDRIVER_Mask 2
```
If you are wanting to view traces only: 
```
ed nt!Kd_IHVDRIVER_Mask 4
```
If you are wanting to view info only:
```
ed nt!Kd_IHVDRIVER_Mask 8
```
If you are wanting to view ALL the info:
```
ed nt!Kd_IHVDRIVER_Mask 8 | 4 | 2
```

After that is done, you will use Gizmo to interact with the driver:
```
Gizmo.exe --hook NtDeviceIoControlFile
```

or multiple hooks at the same time:
```
Gizmo.exe --hook NtDeviceIoControlFile NtCreateFile
```

![Gizmo](https://github.com/ch3rn0byl/Gremlins/blob/master/Images/Untitled.png)

After placing the hook...

![Stripe](https://github.com/ch3rn0byl/Gremlins/blob/master/Images/Screenshot%202022-12-16%20133033.png)

## TODO:
- [ ] Implement the capability to monitor specific devices
- [ ] Implement some fuzzing logic
- [ ] Fix up Gizmo a lil bit
- [ ] Implement other syscalls
