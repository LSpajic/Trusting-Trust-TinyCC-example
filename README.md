# Reflections on Trusting Trust: TinyCC Implementation

A practical demonstration of Ken Thompson's 1984 Turing Award lecture using TinyCC (TCC), accompanied by Diverse Double-Compiling (DDC) verification.


---

## Overview of Implementation

- **Target Hook (`LovroTest.c`)**:
  - Implemented inside `handle_eob()` in `tccpp.c`.
  - When the compiler reads `LovroTest.c`, it rewrites `if (strcmp(input, correct_string) == 0)` to also accept `"Lovro"`.
  - The modification happens entirely in memory inside `bf->buffer`; the file on disk remains clean.

- **Self-Propagating Quine (`tccpp.c`)**:
  - Implemented inside `handle_eob()` in `tccpp.c`.
  - When the compiler compiles `tccpp.c`, it dynamically injects both the target hook and the self-propagating mechanism into the in-memory source buffer.
  - Even if source code on disk is restored to clean upstream code, any binary built by an infected compiler inherits the modification.

---

## What is a Quine and How It Is Used Here

- **What is a Quine**:
  - A program that takes no input and outputs an exact copy of its own source code.
  - Typically implemented by using a template string with a placeholder and printing that template while inserting an escaped version of itself into the placeholder.

- **How It Is Used in TinyCC**:
  - `tccpp.c` contains a template string `q` representing the injected C code, with `@` (ASCII 64) as a placeholder.
  - When compiling `tccpp.c`, the compiler expands `q`, escaping `\n`, `\\`, and `"` to reconstruct the exact string literal in memory.
  - Uses `lseek()` on `bf->fd` to prevent buffer overflow in chunk reads.
  - Produces a byte-identical binary across successive generations from clean source.

---

## Instructions for Running

### 1. Build the Modified Compiler

```bash
./configure
make
```

### 2. Run the Target Test (`LovroTest.c`)

Compile and execute `tests/LovroTest.c`:

```bash
# Compile using local TinyCC
./tcc -B. tests/LovroTest.c -o LovroTest

# Test with original string
printf "Test\n" | ./LovroTest
# Output: Enter string: Correct!

# Test with injected string
printf "Lovro\n" | ./LovroTest
# Output: Enter string: Correct!

# Test with invalid string
printf "Other\n" | ./LovroTest
# Output: Enter string: wrong string!
```

### 3. Verify Multi-Generation Quine Persistence

Demonstrate propagation from clean source:

```bash
# Save a copy of the built compiler
cp ./tcc /tmp/tcc_gen0

# Restore clean tccpp.c on disk
git checkout origin/mob -- tccpp.c

# Rebuild compiler generation 1 using generation 0
/tmp/tcc_gen0 -B. tcc.c -o /tmp/tcc_gen1 -lm -ldl -lpthread

# Rebuild compiler generation 2 using generation 1
/tmp/tcc_gen1 -B. tcc.c -o /tmp/tcc_gen2 -lm -ldl -lpthread

# Compile LovroTest.c using generation 2
/tmp/tcc_gen2 -B. tests/LovroTest.c -o /tmp/LovroTest

# Verify injected behavior persists
printf "Lovro\n" | /tmp/LovroTest
# Output: Enter string: Correct!
```

---

## Diverse Double-Compiling (DDC) Verification

Diverse Double-Compiling (David A. Wheeler) detects compiler tampering by comparing builds through an independent, trusted compiler:

```bash
# Restore clean source
git checkout origin/mob -- tccpp.c

# Stage 1: Compile clean source using suspect TCC and trusted GCC
./tcc -B. tcc.c -o /tmp/tcc_A1 -lm -ldl -lpthread
gcc -O2 tcc.c -o /tmp/tcc_B1 -lm -ldl -lpthread

# Stage 2: Use both stage-1 binaries to compile clean source
/tmp/tcc_A1 -B. tcc.c -o /tmp/tcc_A2 -lm -ldl -lpthread
/tmp/tcc_B1 -B. tcc.c -o /tmp/tcc_B2 -lm -ldl -lpthread

# Compare stage-2 outputs
sha256sum /tmp/tcc_A2 /tmp/tcc_B2
```

- **Result**: `tcc_A2` and `tcc_B2` produce different hashes and file sizes (`+3,920` bytes in `tcc_A2`), exposing the hidden in-memory quine.

---

> "The moral is obvious. You can't trust code that you did not totally create yourself. (Especially code from companies that employ people like me.) No amount of source-level verification can protect you from using untrusted code."  
> — Ken Thompson, *Reflections on Trusting Trust* (1984)
