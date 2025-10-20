# mysh — Minimal Unix‑like Shell (COMP-3659 Project)

This repository contains a minimal Unix‑like shell called **mysh**. It implements a tiny userland with custom memory management, a small string library, a tokenizer/parser, and a two‑stage pipeline executor that supports I/O redirection and background jobs.

---

## Features

- **Custom heap allocator** used for *all* dynamic strings (tokens, paths). Memory is released in one shot between commands via `free_all()`. 
- **String helpers**: `mystrlen`, `mystrcmp`, `mystrcpy`, `mystrdup(begin,end)`, `mystrcat`. These avoid libc calls and integrate with the custom heap. 
- **Tokenizer** that splits words/operators (`|`, `<`, `>`, `&`) and builds `Command.argv[]` with a hard cap to prevent overflow. 
- **Parser** that:
  - splits a single `|` into a two‑stage pipeline (`pipeline[0]`, `pipeline[1]`), rejects multiple pipes, and compacts argv. 
  - handles `< infile` on stage 0 and `> outfile` on the last stage (stage 0 or 1). 
  - detects trailing `&` to run in background. 
- **Launcher/executor** that sets up redirections, optional pipe, and forks stage 0 and stage 1 as needed using `execve()`. Foreground jobs are waited; background jobs return immediately. 
- **Path resolution**: If `argv[0]` contains no `/`, the shell prepends `/usr/bin/` (no `execvp`). 

> The entry point loops: prompt → read → tokenize/parse → run → `free_all()` per command. 

---

## Source Layout

- `mysh.c` — main loop: builds `Job`, calls `get_job()` and `run_job()`, then frees heap. 
- `jobs.h / jobs.c` — data models (`Command`, `Job`, `FD`) and all parsing/launching helpers (pipeline, redirection, background, waiting). 
- `myfunctions.h / myfunctions.c` — tokenizer (`tokenize_command/word/operator`), path resolution, redirection helpers, `get_command()`, `run_command()`. 
- `myheap.h / myheap.c` — linear bump allocator (`alloc`, `free_all`).
- `mystring.h / mystring.c` — basic string utilities used throughout. 
---

## Build

### Prereqs
- POSIX environment (Linux/macOS)
- `gcc` and standard headers

### Compile
```bash
gcc mysh.o mystring.o myheap.o myfunctions.o jobs.o -o mysh
```

---

## Run

```bash
./mysh
```
You’ll see the prompt:
```text
mysh $ 
```

Type commands as you would in a simple shell (see **Examples** below).

---

## Supported Syntax

- **Simple command**: `ls -l /` (executes a single program). 
- **Pipeline (one `|`)**: `ls | wc -l` (splits tokens across two stages). 
- **Input redirection**: `cat < input.txt` (stage 0 only). 
- **Output redirection**: `echo hi > out.txt` (last stage). 
- **Background**: `sleep 5 &` (don’t wait in parent). 
- **Exit**: `exit`. 

Notes & limits:
- Only **one** pipe is supported; multiple pipes are rejected. 
- `argv` length is capped (`MAX_ARGS=16`), input line capped (`MAX_CH=256`). 
- `execve()` is used directly; environment is not forwarded (currently `NULL`).
- Path resolution is hard‑coded to `/usr/bin/` when no `/` in `argv[0]`. 

---

## Testing

This section provides **manual**, **scripted**, and **unit‑style** checks you can run quickly after any change.

### 1) Smoke tests (manual)

From the repo root after building:
```bash
./mysh << 'EOF'
exit
EOF
```
Expected: prints prompt, then `Exiting shell...` and terminates.

Run an unknown command:
```bash
./mysh << 'EOF'
does-not-exist
exit
EOF
```
Expected: `execve failed, please re-enter command`. 

### 2) Simple commands

```bash
./mysh << 'EOF'
/usr/bin/echo hello
echo world
exit
EOF
```
- First line executes absolute path.
- Second line triggers path resolution to `/usr/bin/echo`. Both should print.

### 3) Redirection

Create a file and read it:
```bash
printf 'A
B
C
' > in.txt
./mysh << 'EOF'
cat < in.txt
exit
EOF
```
Expected: prints `A`, `B`, `C`. Input redirection handled on stage 0. 

Write output:
```bash
./mysh << 'EOF'
echo hi > out.txt
exit
EOF
cat out.txt
```
Expected: file contains `hi` with newline. Last stage redirects to file.

### 4) Pipeline

```bash
./mysh << 'EOF'
ls | wc -l
exit
EOF
```
Expected: integer line count; only one `|` allowed. 

Check error on multiple pipes:
```bash
./mysh << 'EOF'
echo a | tr a A | wc -c
exit
EOF
```
Expected: error `only one '|' operator supported`. 

### 5) Background execution

```bash
time ./mysh << 'EOF'
sleep 1 &
exit
EOF
```
Expected: shell returns immediately (no wait on background). 

### 6) Tokenization/limits

- Leading/trailing/mid whitespace should not confuse tokens. 
- Over‑long input (`> MAX_CH`) should flush remainder and reprompt: try a 300‑char line. 

### 7) Unit‑style checks for mystring.c functions

- `mystrlen("abc") == 3` 
- `mystrcmp("a","a")==0`, `<0`/`>0` on different inputs
- `mystrcpy(buf,"x")` writes `"x\0"` 
- `mystrdup("hello"+1,"hello"+4)` yields `"ell"` (and is NUL‑terminated). 
- `mystrcat(dest,"tail")` appends and NUL‑terminates. 

## Usage Examples

```
mysh $ ls -l
mysh $ cat < notes.txt
mysh $ echo "hello" > hello.txt
mysh $ dmesg | tail -20
mysh $ sleep 10 &
mysh $ exit
```

---

## Known Limitations / TODO

This project intentionally implements a minimal feature set to meet COMP 3659 requirements.
A full discussion of these issues appears in the Governance Document – mysh.

-Pipeline Depth — Supports a maximum of two-stage pipelines (cmd1 | cmd2). Multi-stage (cmd1 | cmd2 | cmd3 ...) not implemented.

-Path Resolution — resolve_path() hard-codes /usr/bin/; commands outside this directory must be entered with full or relative paths.

-Signal Handling — Signals like Ctrl+C and Ctrl+Z are not handled. Foreground jobs cannot be interrupted gracefully; background jobs are not tracked afterward.

-Zombie Processes — Background jobs may persist as zombies until the shell exits.

-Error Handling — Some malformed inputs (e.g., misplaced & or nested redirects) may not be validated.

-Built-ins and Environment — Commands such as cd, pwd, jobs, and fg/bg are not implemented. Environment variables and PATH search are not supported.

-Redirection Constraints — < and > must appear in correct positions; complex cases may behave unpredictably.

-User Feedback — Errors print simple messages via write() to stderr; no advanced diagnostics are provided.
---

## Implementation Notes

- The `Job` and `FD` structures document the execution plan and descriptors per stage. 
- `setup_redirection()` opens files; per‑stage launchers compute the correct stdin/stdout to pass to `run_command()`. Parent closes     unused ends to avoid FD leaks.  
- After each command line, the shell calls `free_all()` to reset the bump allocator and avoid leaks across iterations. 

---

## License

Educational use for the course project.
