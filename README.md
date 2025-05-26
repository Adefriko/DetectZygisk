# DetectZygisk
A POC to detect zygisk

Idea of detection is taken from this github issue https://github.com/PerformanC/ReZygisk/issues/171

Writing here in case issue is deleted or removed - 

---------------------------------------------------------------------------------------------------------------------------------------------

#Description

YONO SBI forks a child process where it will do something like this:

```cpp
ptrace(PTRACE_ATTACH, getppid(), 0, 0);
int status;
waitpid(getppid(), &status, 0);
unsigned long msg = 0;
ptrace(PTRACE_GETEVENTMSG, getppid(), 0, &msg);
```

If ReZygisk is active, then msg will be the pid of zygote. I think this is because when init forks into zygote, Linux will put the pid of zygote as the ptrace message of the ptrace fork stop event. Then apparently in the absence of another ptrace stop that would overwrite it, it will propagate into the child processes. YONO SBI then detects this leftover value.

Running something like this just once on zygote makes YONO SBI work fine on my device:
```cpp
ptrace(PTRACE_ATTACH, zygote_pid, 0, 0);
int status;
waitpid(zygote_pid, &status, 0);
ptrace(PTRACE_SYSCALL, zygote_pid, 0, 0);
waitpid(zygote_pid, &status, 0);
ptrace(PTRACE_DETACH, zygote_pid, 0, 0);
```
The syscall ptrace stop will set the ptrace event message to zero.

---------------------------------------------------------------------------------------------------------------------------------------------
