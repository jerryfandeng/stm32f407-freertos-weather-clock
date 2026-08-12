# Project Memory

Last updated: 2026-07-10

## Project Identity

- **Project name**: my_freertos
- **Board**: STM32F407
- **Toolchain**: Keil MDK-ARM (µVision), .uvprojx build
- **Technologies**: FreeRTOS, STM32 HAL, CMSIS
- **Language**: C (snake_case naming, K&R braces, 4-space indent)

## Architecture Notes

_To be filled as work progresses._

## Decisions & Rationale

_To be filled as decisions are made._

## Known Gotchas

_To be filled as issues are encountered._

## External Resources

_To be filled as relevant locations are discovered._

## Current State

_To be filled as work progresses._
## 2026-07-20 — Watchdog (IWDG) 学习

### 新增模块
- driver/wdg/wdg.h / driver/wdg/wdg.c — IWDG 封装驱动
  - wdg_init(wdg_timeout_t) — 使能 LSI → 配置预分频+重装载 → 启动 IWDG
  - wdg_feed() — 重装载计数器
- driver/wdg/wdg_demo.h / driver/wdg/wdg_demo.c — 喂狗演示任务
  - 创建低优任务 wdg_demo，每 1s 喂一次，5 次后停喂 → IWDG 触发复位
- pp/main.c — 在 main_init 末尾调用 wdg_demo_start()

### IWDG 关键参数 (STM32F407)
- 时钟源：LSI (~32kHz)，独立于系统时钟
- 预分频：4/8/16/32/64/128/256
- 重装载：12-bit (0-4095)
- 超时 = prescaler × reload / 32000 (秒)
- 默认预设：2s/5s/10s/20s
- 使能 IWDG 前必须先开 LSI：RCC_LSICmd(ENABLE) + 等 RCC_FLAG_LSIRDY

### 喂狗模式 (FreeRTOS)
- 低优任务喂狗：高优任务挂死 → 低优抢不到 CPU → 不喂 → WDT 复位
- 本项目 demo 采用最简单的单任务喂狗模式

### 2026-07-21 EasyLogger 日志系统改造
- 36 处 printf 转为 elog_i/e/w/d

## 2026-08-12 - README created (session notes)

- Created README.md for GitHub upload based on repo inspection + user draft.
- Actual driver modules: aht20, console, esp_at, rtc, st7789, tim_delay only.
  AGENTS.md module tree is outdated (mentions bl24c512/cpu_tick/key/led/wdg that do not exist).
- Bug FIXED 2026-08-12: app.c xTimerChangePeriod() in wifi_update / time_update
  / inner_update / outdoor_update now uses each timer's own handle.
- Bug FIXED 2026-08-12: elog_port_output_lock() now calls xSemaphoreTake();
  unlock still gives, so log output is actually protected by the mutex.
- Gotcha: UV4.exe -b hangs with no log when another UV4 GUI instance is
  already open (single-instance); close GUI or build via F7 instead.
- Backtrace coverage gap: vApplicationMallocFailedHook() is an empty
  while(1), so heap exhaustion gives no diagnostics; add cm_backtrace_assert().
- configCHECK_FOR_STACK_OVERFLOW = 1 (method 1); method 2 detects overflow
  earlier but backtrace after overflow is still best-effort.
- README now has a commented <video> embed example (docs/demo.mp4) and an
  "开源组件" attribution section for FreeRTOS / EasyLogger / cm_backtrace.
- 2026-08-12: WIFI_SSID / WIFI_PASSWD in app/app.h replaced with placeholders
  (YOUR_WIFI_SSID / YOUR_WIFI_PASSWORD) before public GitHub upload.
- Secret risk: app/app.h contains real WIFI_SSID/WIFI_PASSWD; app/app.c contains
  a seniverse API key in the URL. Do not push to a public GitHub repo as-is.
- 2026-08-12 (2nd session): user confirmed WiFi module is ESP32-C3 (ESP-AT
  firmware). README + AGENTS.md updated; .gitignore added.
- welcome_page_display() is defined but never called in the main flow.
