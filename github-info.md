# Работа с проектом STM32 Logic Analyzer через GitHub

**Репозиторий:** https://github.com/kvas00/stm32-logic-analyzer

---

## 📥 Клонирование проекта на новый компьютер

### Вариант 1: HTTPS (с токеном)

```bash
# Клонировать репозиторий
git clone https://github.com/kvas00/stm32-logic-analyzer.git

# Перейти в директорию проекта
cd stm32-logic-analyzer

# Настроить Git пользователя
git config user.name "Victor"
git config user.email "kvas00@gmail.com"
```

### Вариант 2: SSH (рекомендуется для постоянной работы)

```bash
# Клонировать через SSH
git clone git@github.com:kvas00/stm32-logic-analyzer.git

# Перейти в директорию проекта
cd stm32-logic-analyzer

# Настроить Git пользователя
git config user.name "Victor"
git config user.email "kvas00@gmail.com"
```

---

## 🔧 Настройка окружения для разработки

### 1. Установить необходимые инструменты

**На Linux (Ubuntu/Debian):**
```bash
# Компилятор ARM GCC
sudo apt install gcc-arm-none-eabi

# CMake и Ninja
sudo apt install cmake ninja-build

# ST-Link для прошивки
sudo apt install stlink-tools

# Опционально: OpenOCD
sudo apt install openocd
```

**На Windows:**
- [ARM GCC Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
- [CMake](https://cmake.org/download/)
- [Ninja](https://github.com/ninja-build/ninja/releases)
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)

**На macOS:**
```bash
brew install --cask gcc-arm-embedded
brew install cmake ninja
brew install stlink
```

### 2. Проверить установку

```bash
arm-none-eabi-gcc --version
cmake --version
ninja --version
st-flash --version
```

---

## 🛠️ Сборка проекта

### Первая сборка

```bash
# Создать директорию для сборки
mkdir build
cd build

# Сгенерировать файлы сборки с помощью CMake
cmake .. -G Ninja

# Или использовать пресет (если есть)
cmake --preset=default

# Собрать проект
ninja

# Или через CMake
cmake --build .
```

### Пересборка после изменений

```bash
# Из директории build
ninja

# Или из корня проекта
ninja -C build
```

### Очистка и полная пересборка

```bash
# Удалить build директорию
rm -rf build

# Пересоздать и собрать заново
mkdir build
cd build
cmake .. -G Ninja
ninja
```

---

## 📤 Прошивка в микроконтроллер

### Через ST-Link (Linux/macOS)

```bash
# Из директории build
st-flash write logic-analyzer.bin 0x8000000

# Или указать полный путь
st-flash write build/logic-analyzer.bin 0x8000000
```

### Через OpenOCD

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/encoder-only.elf verify reset exit"
```

### Через STM32CubeProgrammer (GUI - Windows/Linux)

1. Открыть STM32CubeProgrammer
2. Connect → ST-LINK
3. Open file → выбрать `build/encoder-only.hex` или `encoder-only.bin`
4. Download → Start Programming

---

## 🔄 Работа с Git

### Первая настройка после клонирования

```bash
# Проверить текущие настройки
git config --list

# Настроить имя и email (локально для проекта)
git config user.name "Victor"
git config user.email "kvas00@gmail.com"

# Или глобально для всех проектов
git config --global user.name "Victor"
git config --global user.email "kvas00@gmail.com"
```

### Обновление кода с GitHub

```bash
# Получить последние изменения
git pull origin main

# Или с перебазированием
git pull --rebase origin main
```

### Сохранение изменений и отправка на GitHub

```bash
# Проверить статус
git status

# Добавить все измененные файлы
git add .

# Создать коммит
git commit -m "Описание изменений"

# Отправить на GitHub
git push origin main
```

### Работа с ветками

```bash
# Создать новую ветку для фичи
git checkout -b feature/new-functionality

# Переключиться обратно на main
git checkout main

# Слить ветку в main
git merge feature/new-functionality

# Удалить ветку после слияния
git branch -d feature/new-functionality
```

---

## 📂 Структура проекта

```
stm32-logic-analyzer/
├── BACKUP/                    # Резервные копии версий
│   ├── Ver_1.0-pulse-zoom/   # Последняя стабильная версия
│   ├── Version_0.1/          # Базовая версия
│   ├── v0.3-oled/            # Версия с OLED
│   └── v0.4-date-time/       # Версия с RTC
├── Core/
│   ├── Inc/                  # Заголовочные файлы
│   ├── Lib/                  # C++ классы (Encoder, Led, Oled, Tasks)
│   └── Src/                  # Основной код (main.cpp, драйверы)
├── Drivers/                  # HAL драйверы STM32
├── Middlewares/              # FreeRTOS
├── USB_DEVICE/               # USB CDC
├── build/                    # Директория сборки (не в Git)
├── CMakeLists.txt            # Конфигурация CMake
├── .gitignore                # Исключения для Git
└── README.md / CHANGELOG.md  # Документация
```

---

## 🔑 Аутентификация на GitHub

### Использование Personal Access Token (PAT)

1. Создайте токен на GitHub:
   - https://github.com/settings/tokens
   - Generate new token (classic)
   - Выберите scope: `repo`
   - Скопируйте токен

2. При первом push введите:
   - Username: `kvas00`
   - Password: `<ваш_токен>`

3. Сохраните credentials:
```bash
git config credential.helper store
```

### Использование SSH (рекомендуется)

1. Сгенерируйте SSH ключ (если нет):
```bash
ssh-keygen -t ed25519 -C "kvas00@gmail.com"
```

2. Добавьте ключ на GitHub:
```bash
# Скопировать публичный ключ
cat ~/.ssh/id_ed25519.pub

# Вставить на https://github.com/settings/keys
```

3. Переключите URL на SSH:
```bash
git remote set-url origin git@github.com:kvas00/stm32-logic-analyzer.git
```

---

## 📦 Восстановление конкретной версии

### Откат к стабильной версии из BACKUP

```bash
# Восстановить Ver 1.0-pulse-zoom
cp -r BACKUP/Ver_1.0-pulse-zoom/Core/Lib/* Core/Lib/
cp -r BACKUP/Ver_1.0-pulse-zoom/Core/Src/main.cpp Core/Src/

# Пересобрать
ninja -C build

# Прошить
st-flash write build/encoder-only.bin 0x8000000
```

### Откат к конкретному коммиту

```bash
# Посмотреть историю
git log --oneline

# Откатиться к коммиту (например 7851c6a)
git checkout 7851c6a

# Вернуться к последней версии
git checkout main
```

---

## 🐛 Решение проблем

### Проблема: Build директория не создается

```bash
# Убедитесь что CMake установлен
cmake --version

# Создайте директорию вручную
mkdir -p build
cd build
cmake .. -G Ninja
```

### Проблема: Не находится компилятор ARM GCC

```bash
# Проверьте PATH
which arm-none-eabi-gcc

# Добавьте в ~/.bashrc или ~/.zshrc
export PATH=$PATH:/path/to/arm-gcc/bin
```

### Проблема: Git push требует пароль каждый раз

```bash
# Сохраните credentials
git config credential.helper store

# Или используйте SSH (см. раздел выше)
```

### Проблема: ST-Link не находит устройство

```bash
# Проверьте подключение
st-info --probe

# На Linux может потребоваться udev правило
sudo cp /usr/share/stlink/stlink-blackmagic.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

---

## 📖 Дополнительные ресурсы

- **GitHub репозиторий:** https://github.com/kvas00/stm32-logic-analyzer
- **STM32F401 Datasheet:** https://www.st.com/resource/en/datasheet/stm32f401cc.pdf
- **Git документация:** https://git-scm.com/doc
- **CMake документация:** https://cmake.org/documentation/

---

## 📝 Быстрый старт (TL;DR)

```bash
# 1. Клонировать
git clone https://github.com/kvas00/stm32-logic-analyzer.git
cd stm32-logic-analyzer

# 2. Настроить Git
git config user.name "Victor"
git config user.email "kvas00@gmail.com"

# 3. Собрать
mkdir build && cd build
cmake .. -G Ninja
ninja

# 4. Прошить
st-flash write encoder-only.bin 0x8000000

# 5. Работать с изменениями
# ... делаем изменения ...
git add .
git commit -m "Описание изменений"
git push origin main
```

---

**Версия документа:** 1.0
**Последнее обновление:** 2025-12-04
**Автор:** Claude Code + Victor
