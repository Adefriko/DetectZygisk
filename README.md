# DetectZygisk
A POC to detect zygisk

Idea of detection is taken from this github issue https://github.com/PerformanC/ReZygisk/issues/171

Writing here in case issue is deleted or removed - 

---------------------------------------------------------------------------------------------------------------------------------------------

# Description

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


# Implementation

Core idea is written by ChatGPT deep research ( Good use of AI )

```sh
DetectZygisk: JNI function called - starting detection
DetectZygisk: Starting Zygisk detection
DetectZygisk: fork() returned: 22137
DetectZygisk: Parent process waiting for child: 22137
DetectZygisk: fork() returned: 0
DetectZygisk: Child process started
DetectZygisk: Parent PID: 22098
DetectZygisk: Attempting ptrace(PTRACE_ATTACH, 22098, 0, 0)
DetectZygisk: ptrace(PTRACE_ATTACH) successful
DetectZygisk: Calling waitpid(22098, &status, 0)
DetectZygisk: waitpid() successful, status: 4991
DetectZygisk: Calling ptrace(PTRACE_GETEVENTMSG, 22098, 0, &msg)
DetectZygisk: ptrace(PTRACE_GETEVENTMSG) successful, msg: 1456
DetectZygisk: Calling ptrace(PTRACE_DETACH, 22098, 0, 0)
DetectZygisk: ptrace(PTRACE_DETACH) completed
DetectZygisk: Zygisk DETECTED - ptrace have zygote64 pid: 1456
DetectZygisk: waitpid returned: 22137, status: 256
DetectZygisk: Child exited normally with exit code: 1
DetectZygisk: Final detection result: ZYGISK DETECTED
DetectZygisk: Returning: Detected Zygisk
```

# Main Point 

**Using this method, it is also possible for an unprivileged application to retrieve the PID of the zygote64 process, which is typically considered a restricted system process**


# Testing

https://github.com/PerformanC/ReZygisk detected.

