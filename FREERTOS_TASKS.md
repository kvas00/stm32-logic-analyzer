# FreeRTOS Tasks Documentation

## Обзор

Проект использует FreeRTOS для многозадачности. Реализованы две пользовательские задачи плюс задача по умолчанию от CubeMX.

## Задачи

### 1. ledTask (Heartbeat)
**Файл:** [Core/Lib/Tasks.cpp](Core/Lib/Tasks.cpp)
**Приоритет:** Low
**Размер стека:** 512 байт
**Период:** 500 мс

**Назначение:**
Простая индикация работы системы. Переключает состояние LED каждые 500 мс.

**Функция:**
```cpp
void ledTask(void* argument);
```

### 2. testTask (Encoder Testing)
**Файл:** [Core/Lib/Tasks.cpp](Core/Lib/Tasks.cpp)
**Приоритет:** Normal
**Размер стека:** 1024 байт
**Период:** 10 мс (частота опроса 100 Гц)

**Назначение:**
Тестирование энкодера и вывод событий в лог.

**Обрабатываемые события:**
- Нажатие/отпускание кнопки
- Длинное нажатие (1 сек) с автоматическим сбросом счетчика
- Вращение CW/CCW с выводом delta и текущей позиции

**Функция:**
```cpp
void testTask(void* argument);
```

### 3. defaultTask (CubeMX Default)
**Файл:** [Core/Src/main.cpp](Core/Src/main.cpp)
**Приоритет:** Normal
**Размер стека:** 512 байт

**Назначение:**
Задача по умолчанию, создаваемая CubeMX. Можно использовать для других целей.

## Общие ресурсы

### Глобальные указатели
```cpp
extern Led* g_led;         // Указатель на объект LED
extern Encoder* g_encoder; // Указатель на объект Encoder
```

Эти указатели инициализируются в `main()` перед запуском планировщика и доступны всем задачам.

### Функция логирования
```cpp
void Log_Printf(const char* format, ...);
```
Выводит форматированные сообщения параллельно в UART1 и USB-CDC.

## Библиотека OLED

### Файлы
- **C++ wrapper:** [Core/Lib/Oled.hpp](Core/Lib/Oled.hpp), [Core/Lib/Oled.cpp](Core/Lib/Oled.cpp)
- **C driver:** [Core/Inc/sh1106.h](Core/Inc/sh1106.h), [Core/Src/sh1106.c](Core/Src/sh1106.c)
- **Font:** [Core/Inc/sh1106_font.h](Core/Inc/sh1106_font.h), [Core/Src/sh1106_font.c](Core/Src/sh1106_font.c)

### Использование

```cpp
#include "Oled.hpp"

using namespace display;

// В main() или в задаче инициализации
extern I2C_HandleTypeDef hi2c1;  // Предполагается, что I2C1 настроен в CubeMX

Oled* oled = new Oled(&hi2c1);
if (oled->init()) {
    oled->clear();
    oled->drawString(0, 0, "Hello World!");
    oled->update();
}
```

### Основные методы

- `init()` - инициализация дисплея
- `clear()` - очистить экран
- `fill()` - заполнить экран
- `drawString(x, y, str)` - вывести строку
- `drawChar(x, y, ch)` - вывести символ
- `setPixel(x, y, color)` - установить пиксель
- `update()` - обновить дисплей из буфера
- `setContrast(value)` - установить контрастность
- `invertDisplay(invert)` - инвертировать цвета

## Синхронизация

На данный момент задачи не используют механизмы синхронизации (мьютексы, семафоры), так как:
- `ledTask` работает только с LED
- `testTask` работает только с энкодером
- Нет общих критических ресурсов

При добавлении OLED или других общих ресурсов потребуется добавить мьютексы.

## Приоритеты задач

1. **Low:** ledTask (heartbeat) - некритичная индикация
2. **Normal:** testTask, defaultTask - основная логика
3. **High:** (зарезервировано для критичных задач)

## Добавление новой задачи

1. Объявить функцию задачи в `Tasks.h`:
```cpp
void myTask(void* argument);
```

2. Реализовать в `Tasks.cpp`:
```cpp
void myTask(void* argument) {
    for(;;) {
        // Логика задачи
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

3. Создать задачу в `main.cpp`:
```cpp
const osThreadAttr_t myTask_attributes = {
  .name = "myTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadNew(myTask, NULL, &myTask_attributes);
```

## Конфигурация FreeRTOS

Настройки FreeRTOS определены в CubeMX. Основные параметры:
- Heap: heap_4 (рекомендуется для динамического выделения памяти)
- Tick rate: 1000 Hz (1 мс)
- Preemption: включено
- Time slicing: включено

## Отладка

Для отладки задач можно использовать:
- `Log_Printf()` для вывода отладочной информации
- FreeRTOS-aware debugging в IDE (например, Task List в STM32CubeIDE)
- LED индикацию для визуальной диагностики

## Примечания

- После запуска планировщика (`osKernelStart()`) управление программой не возвращается в `main()`
- Весь код после `osKernelStart()` не выполняется
- Функция `HAL_Delay()` заменена на `vTaskDelay()` в контексте RTOS
