# Linter-Surelog

**Linter-Surelog** — линтер для языка SystemVerilog, созданный на основе [Surelog](https://github.com/alainmarcel/Surelog).  
Проект предназначен для статического анализа кода и проверки соблюдения правил и стандартов.

---

**Реализованные проверки**

- FATAL_SYSTEM_TASK_FIRST_ARGUMENT
- CLASS_VARIABLE_LIFETIME
- IMPLICIT_DATA_TYPE_IN_DECLARATION
- PARAMETER_DYNAMIC_ARRAY
- HIERARCHICAL_INTERFACE_IDENTIFIER
- PROTOTYPE_RETURN_DATA_TYPE
- DPI_DECLARATION_STRING
- REPETITION_IN_SEQUENCE
- COVERPOINT_EXPRESSION_TYPE
- COVERGROUP_EXPRESSION
- CONCATENATION_MULTIPLIER
- PARAMETER_OVERRIDE
- MULTIPLE_DOT_STAR_CONNECTIONS

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
```

### Сборка через консоль
```bash
mkdir build && cd build

make
```
После сборки бинарник `lint` появится в папке `bin`.

### Docker
```bash
docker build -t my-linter .
```
---

## Использование

Для проверки SystemVerilog файлов используйте команду:

### Запуск через консоль
```bash
./bin/lint /path/to/your_file/my_sv_code.sv -nobuiltin
```
### Запуск через docker
```bash
docker run --rm -v /path/to/your/project:/data my-linter lint /data/my_sv_code.sv -nobuiltin
