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

## Usage

### Recommended: via prebuilt Docker image

You can use the prebuilt image published to GitHub Container Registry:

```bash
cd /path/to/sv/file.sv
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/toxiccclub/surelint:latest lint file.sv -nobuiltin
```

---

## Build and run locally

### 1. Clone the repository and initialize submodules

Clone the repository and fetch the bundled Surelog submodule:

```bash
git clone https://github.com/toxiccclub/SureLint.git
cd SureLint

git submodule update --init --recursive
```

### 2. Build from source

The project uses CMake driven by the top-level `Makefile`.  
You will need a C++17-capable compiler, CMake, and `make` installed.

To build the **release** binary, run:

```bash
make release
```

This will configure and build the project into the `build` directory.

After a successful build, the `lint` binary will be located at:

```bash
ls build/bin
```

You should see `lint` among the listed files.


### 3. Run from console (local build)

```bash
./build/bin/lint /path/to/your_file/my_sv_code.sv -nobuiltin
```

You can run `./build/bin/lint --help` to see all available options.
