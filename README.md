# Linter-Surelog

**Linter-Surelog** — линтер для языка SystemVerilog, созданный на основе [Surelog](https://github.com/alainmarcel/Surelog).  
Проект предназначен для статического анализа кода и проверки соблюдения правил и стандартов.

---

## Структура проекта

- `linter/src` – Реализация правил линтера (на данный момент реализовано 8 правил).  
- `linter/include` – Заголовочные файлы линтера.  
- `external/Surelog` – Surelog подключен как submodule и используется для анализа SystemVerilog кода.  

---

## Сборка проекта

Для сборки линтера выполните следующие шаги:

```bash
# Клонируем репозиторий
git clone https://github.com/toxiccclub/Linter_Surelog.git
cd Linter_Surelog

# Инициализация submodule Surelog
git submodule update --init --recursive

# Сборка проекта
make
```

После сборки бинарник `lint` появится в папке `bin`.

---

## Использование

Для проверки SystemVerilog файлов используйте команду:

```bash
./bin/lint /путь/к/файлу/my_sv_code.sv -nobuiltin
```

Пояснения к аргументам:
- `/путь/к/файлу/my_sv_code.sv` – путь к файлу или директории с файлами, которые нужно проверить.   
