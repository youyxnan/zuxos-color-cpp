/* Copyright 2022-2023 John "topjohnwu" Wu
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

// This is the public API for Zygisk modules.
// DO NOT MODIFY ANY CODE IN THIS HEADER.

#pragma once

#include <jni.h>

#define ZYGISK_API_VERSION 5

/*

***************
* Introduction
***************

On Android, all app processes are forked from a special daemon called "Zygote".
For each new app process, zygote will fork a new process and perform "specialization".
This specialization operation enforces the Android security sandbox on the newly forked
process to make sure that 3rd party application code is only loaded after it is being
restricted within a sandbox.

On Android, there is also this special process called "system_server". This single
process hosts a significant portion of system services, which controls how the
Android operating system and apps interact with each other.

The Zygisk framework provides a way to allow developers to build modules and run custom
code before and after system_server and any app processes' specialization.
This enable developers to inject code and alter the behavior of system_server and app processes.

Please note that modules will only be loaded after zygote has forked the child process.
THIS MEANS ALL OF YOUR CODE RUNS IN THE APP/SYSTEM_SERVER PROCESS, NOT THE ZYGOTE DAEMON!

*********************
* Development Guide
*********************

Define a class and inherit zygisk::ModuleBase to implement the functionality of your module.
Use the macro REGISTER_ZYGISK_MODULE(className) to register that class to Zygisk.

Example code:

static jint (*orig_logger_entry_max)(JNIEnv *env);
static jint my_logger_entry_max(JNIEnv *env) { return orig_logger_entry_max(env); }

class ExampleModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        JNINativeMethod methods[] = {
            { "logger_entry_max_payload_native", "()I", (void*) my_logger_entry_max },
        };
        api->hookJniNativeMethods(env, "android/util/Log", methods, 1);
        *(void **) &orig_logger_entry_max = methods[0].fnPtr;
    }
private:
    zygisk::Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(ExampleModule)

-----------------------------------------------------------------------------------------

Since your module class's code runs with either Zygote's privilege in pre[XXX]Specialize,
or runs in the sandbox of the target process in post[XXX]Specialize, the code in your class
never runs in a true superuser environment.

If your module require access to superuser permissions, you can create and register
a root companion handler function. This function runs in a separate root companion
daemon process, and an Unix domain socket is provided to allow you to perform IPC between
your target process and the root companion process.

Example code:

static void example_handler(int socket) { ... }

REGISTER_ZYGISK_COMPANION(example_handler)

*/

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:

    // This method is called as soon as the module is loaded into the target process.
    // A Zygisk API handle will be passed as an argument.
    virtual void onLoad([[maybe_unused]] Api *api, [[maybe_unused]] JNIEnv *env) {}

    // This method is called before the app process is specialized.
    // At this point, the process just got forked from zygote, but no app specific specialization
    // is applied. This means that the process does not have any sandbox restrictions and
    // still runs with the same privilege of zygote.
    //
    // All the arguments that will be sent and used for app specialization is passed as a single
    // AppSpecializeArgs object. You can read and overwrite these arguments to change how the app
    // process will be specialized.
    //
    // If you need to run some operations as superuser, you can call Api::connectCompanion() to
    // get a socket to do IPC calls with a root companion process.
    // See Api::connectCompanion() for more info.
    virtual void preAppSpecialize([[maybe_unused]] AppSpecializeArgs *args) {}

    // This method is called after the app process is specialized.
    // At this point, the process has all sandbox restrictions enabled for t
