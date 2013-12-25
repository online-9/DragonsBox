DragonsBox
==========
Dragons Sandboxie is used to debug and see what a specific program is doing

Works for Native Programs
Works for .Net Programs
Block request
Allow request
Remember the choice u made (block or allow)
When a program wants to do something and it gets detected by DragonsBox a popup will come up with a warning that a program requested something to do, with a little description
When a process is sandboxed the name of the process in the processlist becomes red

Current detections:
Detects if a program wants to screencapture
MessageBox
Keyboard Hook
Mouse hook
13 Window hook Message detections (this includes global hooks such as keylogger etc)
Writing data to a file (You're able to dump the data)
Is able to detect a runPE
WriteProcessMemory detection (Writing data in another process) (You're able to dump the data)
Detects the "Assembly.Load" both in VB/C#
Dumps all the methods both from the .Net Framework and Native(Only the hooked)