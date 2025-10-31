#ifndef BLEPRPH_H
#define BLEPRPH_H
#ifdef __cplusplus
extern "C" {
#endif

// 初始化 BLE 模块
esp_err_t InitBlueTooth(void);
// BLE 任务函数
void AppBlueTooth(void *pvParameters);

#ifdef __cplusplus
}
#endif
#endif // BLEPRPH_H
