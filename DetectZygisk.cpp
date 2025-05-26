#include <jni.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <android/log.h>
#include <errno.h>
#include <stdlib.h>

#define LOG_TAG "DetectZygisk"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool detectZygisk() {
    LOGI("Starting Zygisk detection");

    pid_t child_pid = fork();
    LOGD("fork() returned: %d", child_pid);

    if (child_pid == 0) {
        LOGD("Child process started");

        pid_t parent_pid = getppid();
        LOGD("Parent PID: %d", parent_pid);

        LOGD("Attempting ptrace(PTRACE_ATTACH, %d, 0, 0)", parent_pid);
        if (ptrace(PTRACE_ATTACH, parent_pid, 0, 0) == -1) {
            LOGE("ptrace(PTRACE_ATTACH) failed, errno: %d", errno);
            _exit(0);
        }
        LOGD("ptrace(PTRACE_ATTACH) successful");

        int status;
        LOGD("Calling waitpid(%d, &status, 0)", parent_pid);
        if (waitpid(parent_pid, &status, 0) == -1) {
            LOGE("waitpid() failed, errno: %d", errno);
            ptrace(PTRACE_DETACH, parent_pid, 0, 0);
            _exit(0);
        }
        LOGD("waitpid() successful, status: %d", status);

        unsigned long msg = 0;
        LOGD("Calling ptrace(PTRACE_GETEVENTMSG, %d, 0, &msg)", parent_pid);
        if (ptrace(PTRACE_GETEVENTMSG, parent_pid, 0, &msg) == -1) {
            LOGE("ptrace(PTRACE_GETEVENTMSG) failed, errno: %d", errno);
            ptrace(PTRACE_DETACH, parent_pid, 0, 0);
            _exit(0);
        }
        LOGI("ptrace(PTRACE_GETEVENTMSG) successful, msg: %lu", msg);

        LOGD("Calling ptrace(PTRACE_DETACH, %d, 0, 0)", parent_pid);
        if (ptrace(PTRACE_DETACH, parent_pid, 0, 0) == -1) {
            LOGE("ptrace(PTRACE_DETACH) failed, errno: %d", errno);
        }
        LOGD("ptrace(PTRACE_DETACH) completed");

        if (msg > 0 && msg < 65536) {
            LOGI("Zygisk DETECTED - ptrace have zygote64 pid: %lu", msg);
            _exit(1);
        } else {
            LOGI("Zygisk NOT DETECTED - ptrace message: %lu", msg);
            _exit(0);
        }
    } else if (child_pid > 0) {
        LOGD("Parent process waiting for child: %d", child_pid);

        int status;
        pid_t wait_result = waitpid(child_pid, &status, 0);
        LOGD("waitpid returned: %d, status: %d", wait_result, status);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            LOGD("Child exited normally with exit code: %d", exit_code);
            bool result = (exit_code == 1);
            LOGI("Final detection result: %s", result ? "ZYGISK DETECTED" : "ZYGISK NOT DETECTED");
            return result;
        } else if (WIFSIGNALED(status)) {
            int signal_num = WTERMSIG(status);
            LOGE("Child process killed by signal: %d", signal_num);
            LOGE("Child was terminated abnormally");
        } else {
            LOGE("Child process status unknown: %d", status);
        }
    } else {
        LOGE("fork() failed, errno: %d", errno);
    }

    LOGI("Detection failed or error occurred");
    return false;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_test_detectz_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    LOGI("JNI function called - starting detection");

    if (detectZygisk()) {
        LOGI("Returning: Detected Zygisk");
        return env->NewStringUTF("Zygisk Detected");
    } else {
        LOGI("Returning: Zygisk Not Detected");
        return env->NewStringUTF("Zygisk Not Detected");
    }

}