# Repository Guidelines

## Project Structure & Module Organization

```
my_freertos/
├── app/              # Application layer (pages, main loop, Wi-Fi, weather)
│   ├── font/         # Bitmap font assets (Maple font variants)
│   ├── image/        # Bitmap icon/image assets
│   └── page/         # UI page implementations (welcome, Wi-Fi, main, error)
├── driver/           # Peripheral driver modules (one subdirectory each)
│   ├── aht20/        # Temperature/humidity sensor
│   ├── bl24c512/     # EEPROM
│   ├── console/      # Debug console
│   ├── cpu_tick/     # SysTick timing
│   ├── esp_at/       # ESP32-C3 AT command Wi-Fi module
│   ├── key/          # Button/input driver
│   ├── led/          # LED indicator
│   ├── rtc/          # Real-time clock
│   └── st7789/       # TFT LCD display driver
├── firmware/         # STM32 HAL & CMSIS (vendor-supplied, read-only)
│   ├── cmsis/        # CMSIS-Core (core_cm4.h, etc.)
│   └── driver/       # STM32F4xx peripheral drivers (inc/ + src/)
├── mdk/              # Keil MDK project files and build artifacts
├── tools/            # Python utility scripts (font conversion, etc.)
├── resources/        # Resource assets
├── third_lib/        # Third-party library binaries
├── main.c            # Entry point (reset handler hierarchy)
├── app.h             # Application-level API declarations
├── weather.c/.h      # Weather data fetching and parsing
├── wifi.c            # Wi-Fi connection management
├── mloop.c           # Main event loop
└── board.c           # Board-specific initialization
```

Each driver module under `driver/` follows a consistent pattern: a `.h` header exporting the public API, and a `.c` implementation. Drivers are self-contained and depend only on `firmware/` HAL types.

## Build, Flash, and Development

This project uses **Keil MDK-ARM** (µVision) as the primary build toolchain. The IDE project file is:

- `stm32f407.uvprojx` — project file (open in µVision to build)
- `stm32f407.uvoptx` — per-user options

**Build:** Open `stm32f407.uvprojx` in Keil µVision and press **F7** (Build), or use the toolbar Build button.

**Flash:** Press **F8** (Download) in µVision to flash the target via the configured debug probe (J-Link / ST-Link).

**Utility scripts** are in `tools/`:

```bash
python tools/convert_chinese_font.py  # Regenerate bitmap font assets
```

## Coding Style & Naming Conventions

- **Indentation:** 4 spaces. No tabs.
- **Braces:** K&R style — opening brace on the same line as the control statement.
- **Naming:**
  - Functions and variables: `snake_case` (e.g., `wifi_init`, `main_loop`).
  - Macros and constants: `UPPER_SNAKE_CASE` (e.g., `WIFI_SSID`, `APP_VERSION`).
  - Header guards: `__MODULE_NAME_H__` (e.g., `__APP_H__`).
  - Types: `snake_case_t` for typedefs.
- **Headers:** Each module exposes its public API through a single header. Internal helpers are `static` in the `.c` file.
- No automatic formatter is configured; manually match the style of surrounding code.

## Testing Guidelines

This repository does not yet include automated tests. Testing is performed by flashing the firmware to the STM32F407 target and verifying behavior through the TFT display, serial console output, and peripheral interaction. When adding tests:

- Name test functions `test_<module>_<scenario>`.
- Place host-side test scripts under a `tests/` directory at the repository root.

## Commit & Pull Request Guidelines

**Commit messages:** Follow a conventional format:

```
<module>: <imperative description>

<optional body explaining rationale>
```

Examples:

```
driver/aht20: fix measurement timeout under high humidity
app/weather: add error handling for HTTP response parsing
page: add Wi-Fi connection status indicator
```

**Pull requests:**

- Describe what the change does and why.
- Reference any related issue or hardware behavior.
- Include a description of how the change was tested (which board, peripherals, observed behavior).
- If the PR affects the display, consider adding a photo of the TFT output.
- Keep each PR scoped to one logical change (one driver, one page, one feature).

## Agent-Specific Instructions

This `AGENTS.md` file is read by AI coding agents to understand repository conventions. Agents should:

- Read this file before making changes.
- Respect the module boundary pattern — place new drivers in their own subdirectory under `driver/` with a matching `.h`/`.c` pair.
- Keep `firmware/` read-only — that directory mirrors the vendor HAL and should not be modified.
- Follow the existing naming and indentation style for all new code.
- Test changes by building with the Keil project file before committing.
