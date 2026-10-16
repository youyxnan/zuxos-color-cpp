#include <android/log.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>

#include "zygisk.hpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZuxosColorHook", __VA_ARGS__)

using zygisk::Api;
using zygisk::ServerSpecializeArgs;

class ZuxosColorModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        // 不需要保存 api 和 env，后续逻辑不会用到
    }

    void postServerSpecialize(const ServerSpecializeArgs *args) override {
        LOGD("Zygisk 模块加载成功，准备开始后台监听！");
        
        // 启动独立线程，在系统进程内循环检测亮屏
        std::thread([]() {
            // 延迟5秒，等待系统服务完全就绪
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            std::string last_state = "";
            while (true) {
                // 200毫秒轮询一次，在 system_server 中开销极小，且绝对不会被冻结
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                // 获取屏幕状态
                FILE *fp = popen("getprop sys.screen.state", "r");
                if (fp) {
                    char buf[64];
                    if (fgets(buf, sizeof(buf), fp)) {
                        std::string current_state(buf);
                        // 去除换行符
                        if (!current_state.empty() && current_state.back() == '\n') {
                            current_state.pop_back();
                        }
                        
                        // 如果亮屏，且上一次是熄屏状态
                        if ((current_state == "Awake" || current_state == "ON") && 
                            (last_state != "Awake" && last_state != "ON")) {
                            
                            LOGD("检测到亮屏，开始执行色彩恢复...");
                            // 以 root 权限在后台调用脚本
                            system("su -c 'sh /data/adb/modules/zuxos_custom_color/scripts/apply_conf.sh' &");
                        }
                        last_state = current_state;
                    }
                    pclose(fp);
                }
            }
        }).detach();
    }
};

REGISTER_ZYGISK_MODULE(ZuxosColorModule)
