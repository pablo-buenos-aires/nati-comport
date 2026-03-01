# Watcher (Windows 10, C++Builder XE)

Консольная утилита для мониторинга завершения процесса `TargetApp` и запуска короткого обмена по COM-порту только после события завершения.

## Возможности

- Мониторинг TargetApp:
  - по PID: `--pid <число>`
  - по имени exe: `--process <name.exe>` (берется самый свежий экземпляр)
- Ожидание завершения процесса через `WaitForSingleObject`.
- Классификация выхода:
  - `exitCode == 0` → `NormalExit`
  - `exitCode != 0` → `CrashOrError`
  - нет кода → `UnknownExit`
- Действие после завершения:
  - открыть COM (`--com COM3`), `9600` по умолчанию (`--baud` опционально)
  - 8N1, без hardware flow control
  - отправка `--write` (по умолчанию `PING\r\n`)
  - чтение ответа с таймаутом `--timeout` (по умолчанию 2000 мс), проверка `--expect` (по умолчанию `PONG`)
  - закрытие порта после обмена
- Ретраи открытия COM: 5 попыток c backoff: `200/400/800/1600/2000` мс
- Режимы:
  - `--once` — одно срабатывание и выход
  - без `--once` и с `--process` — бесконечный цикл мониторинга перезапусков
- `--dry-run` — без реального доступа к COM, только лог действий.

## Файлы проекта

- `main.cpp` — основной цикл приложения
- `ProcessMonitor.h/.cpp` — поиск процессов и ожидание завершения
- `SerialPort.h/.cpp` — работа с COM-портом через WinAPI
- `Logger.h/.cpp` — логирование с timestamp и уровнями
- `Cli.h/.cpp` — разбор аргументов командной строки
- `VclCompat.cpp` — shim для C++Builder XE: устраняет линковку `__InitVCL` в console/non-VCL конфигурациях

## Сборка в Embarcadero RAD Studio / C++Builder XE

1. Создайте **Console Application** (C++).
2. Добавьте в проект файлы:
   - `main.cpp`
   - `ProcessMonitor.cpp/.h`
   - `SerialPort.cpp/.h`
   - `Logger.cpp/.h`
   - `Cli.cpp/.h`
3. Убедитесь, что проект собирается под Win32.
4. Build (`Ctrl+F9`).

> Используются только WinAPI и базовый C++ (совместимо с XE).

## Примеры запуска

```bat
watcher.exe --process notepad.exe --com COM3 --dry-run --once
watcher.exe --pid 1234 --com COM3 --timeout 2000
watcher.exe --process TargetApp.exe --com COM3 --write "PING\r\n" --expect "PONG"
```

## Тестирование без COM-устройства

Используйте dry-run:

```bat
watcher.exe --process notepad.exe --com COM3 --dry-run
```

Программа будет логировать, что сделала бы открытие/запись/чтение, не открывая реальный порт.
