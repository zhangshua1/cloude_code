#include "config.h"
#include "balance/imu_manager.h"
#include "balance/balance_task.h"
#include "motor/tb6612_driver.h"
#include "motor/odometry.h"
#include "servo/pca9685_driver.h"
#include "servo/animation.h"
#include "display/display_task.h"
#include "comm/wifi_manager.h"
#include "comm/websocket.h"
#include "comm/protocol.h"
#include "system/battery_monitor.h"
#include "ui/web_ui.h"
#include "audio/audio_capture.h"
#include "ai/ai_service.h"
#include "ai/conversation.h"
#include <esp_task_wdt.h>

// 全局对象
IMUManager  g_imu;
TB6612      g_motors;

// ===================== FreeRTOS 任务 =====================

// 200Hz 平衡控制任务 (Core 1)
static void balanceTaskFunc(void*) {
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(BALANCE_PERIOD_MS);

    while (true) {
        vTaskDelayUntil(&lastWake, period);
        BalanceTask::instance().run();
    }
}

// 50Hz 舵机+里程计
static void servoTimerFunc(void*) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
        PCA9685Driver::instance().update();
        AnimationEngine::instance().update();
        Odometry::instance().update();
    }
}

// 30Hz 显示刷新
static void displayTaskFunc(void*) {
    while (true) {
        DisplayTask::instance().run();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

// 20Hz 遥测推送
static void commTaskFunc(void*) {
    while (true) {
        if (WebSocketServer::instance().hasClients()) {
            JsonDocument doc(512);
            ProtocolHandler::buildTelemetry(doc);
            WebSocketServer::instance().broadcastTelemetry(doc);
        }
        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_MS));
    }
}

// 20Hz 对话状态机
static void conversationTaskFunc(void*) {
    while (true) {
        ConversationTask::instance().run();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// 1Hz 系统监控
static void monitorTaskFunc(void*) {
    while (true) {
        BatteryMonitor::instance().getVoltage();

        if (BatteryMonitor::instance().isCritical()) {
            Logger::instance().warn("Battery critical! %.2fV",
                                    BatteryMonitor::instance().getVoltage());
            BalanceTask::instance().setMode(BalanceMode::IDLE);
        } else if (BatteryMonitor::instance().isLow()) {
            Logger::instance().warn("Battery low: %.2fV",
                                    BatteryMonitor::instance().getVoltage());
        }

        // 喂看门狗
        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ===================== Setup / Loop =====================

void setup() {
    Serial.begin(115200);
    delay(500);
    log_i("=== PenguinBot v%s ===", FIRMWARE_VER);

    // --- 硬件初始化 ---
    if (!g_imu.begin()) {
        log_e("IMU init failed! Halting.");
        while (1) { delay(1000); }
    }
    g_imu.calibrate();

    g_motors.begin();

    if (!PCA9685Driver::instance().begin())
        log_e("PCA9685 init failed");

    BatteryMonitor::instance().begin();
    Odometry::instance().begin(&g_motors);

    // --- 软件模块初始化 ---
    DisplayTask::instance().begin();
    DisplayTask::instance().setExpression(Expression::NEUTRAL);

    BalanceTask::instance().begin(&g_imu, &g_motors);
    AnimationEngine::instance().begin();
    WiFiManager::instance().begin();
    WebSocketServer::instance().begin();
    ProtocolHandler::instance().begin();
    WebUI::instance().begin();

    Logger::instance().begin();
    Logger::instance().info("PenguinBot startup complete");

    // --- AI 对话模块 ---
    if (AudioCapture::instance().begin()) {
        Logger::instance().info("AudioCapture: INMP441 ready");
    }
    AIService::instance().begin();
    ConversationTask::instance().begin();

    // --- 创建任务 ---
    // 平衡任务固定在 Core 1
    xTaskCreatePinnedToCore(balanceTaskFunc, "balance", 4096, NULL,
                            PRIO_BALANCE, NULL, 1);
    xTaskCreate(conversationTaskFunc, "conv",   8192, NULL, PRIO_CONVERSATION, NULL);
    xTaskCreate(servoTimerFunc,       "servo",  3072, NULL, PRIO_SERVO,   NULL);
    xTaskCreate(displayTaskFunc,      "display", 8192, NULL, PRIO_DISPLAY, NULL);
    xTaskCreate(commTaskFunc,         "comm",   8192, NULL, PRIO_COMM,    NULL);
    xTaskCreate(monitorTaskFunc,      "monitor", 2048, NULL, PRIO_MONITOR, NULL);

    // 启动看门狗
    esp_task_wdt_init(3, true);
    esp_task_wdt_add(NULL);

    Logger::instance().info("All tasks started. IP: %s",
                            WiFiManager::instance().getIP().c_str());

    BalanceTask::instance().setMode(BalanceMode::STAND);
}

void loop() {
    // Arduino loop 不执行逻辑，所有工作在 FreeRTOS 任务中
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_task_wdt_reset();
}
