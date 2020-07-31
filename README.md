# Notify Routines Research
A research project about Windows notify routines.

The full research paper is available in:
- English (TBD)
- [Hebrew](https://www.digitalwhisper.co.il/files/Zines/0x78/DW120-7-WinNotificationRoutine.pdf)

## Projects
1. **Remote Thread Shellcode Injector** - A basic code injector which injects shellcode to a remote process using CreateRemoteThread.
1. **Remote Thread Detector Driver** - A driver that detects remote thread creations.
1. **Library Hooking Driver & Dll** - A driver that hooks library functions in every new process using DLL injection from the kernel.
1. **Notify Routine Enumeration Driver** - A driver that enumerates all the (create thread) notify routines currenty registered in the system.
1. **Notify Routine Hooking Driver** - A driver that hooks (create thread) notify routine and bypasses the LibraryHookingDriver.

## License
[MIT](https://choosealicense.com/licenses/mit/)

## Author
- **Omer Katz** - [omerk2511](https://github.com/omerk2511)
