#include <android/log.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>

#include "zygisk.hpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZuxosColorHook", __VA_ARGS__)

using zygisk::Api;
using zygisk::ServerSpecializeArgs;

class ZuxosColorModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postServerSpecialize(const ServerSpecializeArgs *args) override {
        LOGD("Zygisk 模块加载成功，准备开始注入！");
        
        // 【关键】把日志写入文件，方便直接查看
        FILE *fp = fopen("/data/local/tmp/zygisk_debug.log", "a");
        if (fp) {
            fprintf(fp, "[Zygisk] 模块成功注入 system_server！\n");
            fclose(fp);
        }
        
        executeScript();
    }

private:
    Api *api;
    JNIEnv *env;

    void executeScript() {
        // 尝试执行脚本
        int ret = system("sh /data/adb/modules/zuxos_custom_color/scripts/apply_conf.sh &");
        
        // 把执行结果写进文件
        FILE *fp = fopen("/data/local/tmp/zygisk_debug.log", "a");
        if (fp) {
            fprintf(fp, "[Zygisk] 执行 apply_conf.sh 返回结果: %d\n", ret);
            if (ret != 0) {
                fprintf(fp, "[Zygisk] 警告：脚本执行可能失败，可能需要 root 权限！\n");
            }
            fclose(fp);
        }
    }
};

REGISTER_ZYGISK_MODULE(ZuxosColorModule)
