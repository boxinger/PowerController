---
name: "STM32 Firmware Guidelines"
description: "Use when editing STM32 firmware files under Core, User, or App, HAL peripheral setup, interrupt handlers, scheduler logic, or board-level driver code. Covers CubeMX-safe edits, embedded C style, interrupt safety, and validation expectations."
applyTo: "Core/**/*.c, Core/**/*.h, User/**/*.c, User/**/*.h, App/**/*.c, App/**/*.h"
---

# STM32 Firmware Guidelines

Use these instructions when modifying generated STM32 firmware or project-specific embedded C modules.

## Edit Boundaries

- In CubeMX-generated files, prefer changes inside USER CODE blocks when that structure exists.
- Do not rewrite auto-generated initialization flow unless the task explicitly requires it.
- Put reusable project logic in App or User modules instead of expanding main.c indefinitely.

## Implementation Rules

- Keep hardware-facing code explicit and easy to trace.
- Prefer small functions with clear ownership over deeply nested control flow.
- Avoid dynamic memory allocation in firmware code.
- Keep ISR work minimal; defer heavier processing to flags, queues, or scheduled tasks.
- Preserve existing public interfaces unless the task requires an API change.

## Peripheral And State Safety

- Check timer, DMA, ADC, UART, and GPIO interactions before changing shared state.
- Treat variables shared with interrupts or DMA as concurrency-sensitive.
- When changing sampling or control timing, review trigger source, period, callback path, and data consumption together.
- Do not add blocking waits in code paths that affect real-time behavior unless the task explicitly calls for it.

## Validation Expectations

- Prefer the narrowest available validation for the touched firmware area.
- For build-affecting changes, run at least a targeted compile or project build when the environment allows it.
- If hardware behavior cannot be verified locally, state that clearly and list the expected on-device checks.

## Response Style

- Explain embedded risks concretely: timing, interrupt ordering, shared peripherals, buffer ownership, and initialization order.
- Keep changes minimal and consistent with the existing STM32 HAL and project structure.