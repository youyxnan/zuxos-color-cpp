#include <android/log.h>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "zygisk.hpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZuxosColorHook", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ZuxosColorHook", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class ZuxosColorModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    // 【关键】这里的参数必须有 const，否则会编译报错
    void postServerSpecialize(const ServerSpecializeArgs *args) override {
        LOGD("Zygisk 模块加载成功，准备开始注入！");
        LOGD("准备执行 apply_conf.sh 恢复色彩...");
        executeScript();
    }

private:
    Api *api;
    JNIEnv *env;

    void executeScript() {
        // 在 system_server (root 权限) 中执行外部 shell 脚本
        // 加 & 让它在后台执行，避免阻塞 system_server 导致系统卡死
        int ret = system("sh /data/adb/modules/zuxos_custom_color/scripts/apply_conf.sh &");
        if (ret == -1) {
            LOGE("执行 apply_conf.sh 失败！");
        } else {
            LOGD("apply_conf.sh 触发成功！");
        }
    }
};

REGISTER_ZYGISK_MODULE(ZuxosColorModule)
