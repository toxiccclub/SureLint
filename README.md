## SureLint

**SureLint** is a SystemVerilog linter built on top of [Surelog](https://github.com/alainmarcel/Surelog).  
The project is intended for static code analysis and checking compliance with coding rules and standards.

---

## Implemented rules

- `FATAL_SYSTEM_TASK_FIRST_ARGUMENT`
- `CLASS_VARIABLE_LIFETIME`
- `IMPLICIT_DATA_TYPE_IN_DECLARATION`
- `PARAMETER_DYNAMIC_ARRAY`
- `HIERARCHICAL_INTERFACE_IDENTIFIER`
- `PROTOTYPE_RETURN_DATA_TYPE`
- `DPI_DECLARATION_STRING`
- `REPETITION_IN_SEQUENCE`
- `COVERPOINT_EXPRESSION_TYPE`
- `COVERGROUP_EXPRESSION`
- `CONCATENATION_MULTIPLIER`
- `PARAMETER_OVERRIDE`
- `MULTIPLE_DOT_STAR_CONNECTIONS`

---

## Project structure

- `linter/src` – implementation of linter rules.  
- `linter/include` – linter header files.  
- `external/Surelog` – Surelog submodule used for SystemVerilog code analysis.  

---

## Building the project

Clone the repository and initialize the Surelog submodule:

```bash
git clone https://github.com/toxiccclub/SureLint.git
cd Linter_Surelog

git submodule update --init --recursive
```

### Build from source

The project uses CMake via the top-level `Makefile`. To build the release binary, run:

```bash
make release
```

After a successful build, the `lint` binary will be located in the `build/bin` directory.

### Docker

```bash
docker build -t my-linter .
```

---

## Usage

### Run from console

```bash
./build/bin/lint /path/to/your_file/my_sv_code.sv -nobuiltin
```

### Run via Docker

```bash
cd /path/to/sv/files
docker run --rm -v "$(pwd)":/data -w /data my-linter lint file.sv -nobuiltin
```
