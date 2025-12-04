# Стиль кода и правила проекта

## Обработка неиспользуемых параметров

### Правило: Явное указание неиспользуемых параметров

Для всех функций, где параметр не используется, добавляем явное указание `(void)parameter`:

```cpp
void myFunction(void* argument) {
    (void)argument;  // Явно указываем, что параметр не используется
    
    // ... остальной код
}
```

### Применение:

✅ **Наш код** (Tasks.cpp, main.cpp):
- Используем `(void)parameter` в каждой функции с неиспользуемыми параметрами
- Компилятор видит, что параметр намеренно не используется
- Код чист и понятен

✅ **Библиотечный код ST** (USB_DEVICE, HAL Drivers):
- Отключаем предупреждения селективно через `set_source_files_properties`
- Не трогаем библиотечный код, так как он генерируется CubeMX

## Volatile переменные

### Правило: Избегать ++ и -- для volatile

Для volatile переменных вместо:
```cpp
volatile int counter;
counter++;  // ⚠️ Deprecated warning в C++20
```

Используем:
```cpp
volatile int counter;
counter = counter + 1;  // ✅ Правильно
```

### Применение в Encoder.cpp:

```cpp
encoder_pos = encoder_pos + 1;  // Вместо encoder_pos++
encoder_pos = encoder_pos - 1;  // Вместо encoder_pos--
```

## Designated Initializers

Используем C++20 designated initializers для ясности:

```cpp
const osThreadAttr_t task_attributes = {
  .name = "myTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
```

Предупреждения `missing-field-initializers` - это нормально для designated initializers, 
так как остальные поля автоматически инициализируются нулями.

## Результат

### Наш код (проект):
- ✅ Нет предупреждений об unused parameters
- ✅ Нет предупреждений о volatile операциях
- ✅ Чистая компиляция наших файлов

### Библиотечный код (ST HAL/USB/FreeRTOS):
- ⚠️ Предупреждения остаются, но отключены селективно
- Библиотечный код не модифицируется
- При регенерации CubeMX настройки сохраняются

## Настройки компилятора

### CMakeLists.txt:
```cmake
# Селективное отключение предупреждений для библиотечного кода
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/App/usbd_cdc_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/Target/usbd_conf.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter"
)
```

## Стандарты

- **C:** C11 (для HAL и библиотек ST)
- **C++:** C++20 (для нашего кода)
- **Предупреждения:** -Wall -Wextra -Wpedantic (максимальный уровень для нашего кода)
