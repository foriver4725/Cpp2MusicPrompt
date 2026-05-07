# Cpp2MusicPrompt

Cpp2MusicPrompt is an experimental project that transforms C++ program structure into music-oriented prompts.

The project analyzes C++ source code using Clang AST, extracts structural programming events such as functions, loops, branches, and variable definitions, then converts them into descriptive musical instructions for AI music generation systems.

---

## Concept

Programming and music both contain:

- structure
- repetition
- tension and release
- motifs
- progression

This project explores the idea of interpreting source code as musical composition data.

For example:

| C++ Structure | Musical Interpretation |
|---|---|
| Function Definition | Introduce a new theme |
| Variable Definition | Add a short motif |
| If Statement | Tense branching progression |
| For Loop | Repetitive rhythm |
| While Loop | Unstable continuation |
| Function Call | Reuse an existing theme |
| Return Statement | Resolution |

---

## Pipeline

```text
C++ Source Code
        ↓
Clang AST Analysis
        ↓
Event Extraction
        ↓
events.json
        ↓
Python Prompt Generator
        ↓
Music Generation Prompt
```

---

## Features

- Clang AST based analysis
- Structural event extraction
- JSON event serialization
- Music-oriented prompt generation
- Easily extendable event system
- Experimental code-to-music workflow

---

## Example

### Input C++ Code

```cpp
int Add(int a, int b) {
    return a + b;
}

int main() {
    int result = Add(10, 20);

    if (result > 20) {
        return 1;
    }

    return 0;
}
```

### Extracted Events

```json
[
    {
        "kind": "FunctionDefinition",
        "name": "Add"
    },
    {
        "kind": "FunctionCall",
        "name": "Add"
    },
    {
        "kind": "IfStatement"
    },
    {
        "kind": "ReturnStatement"
    }
]
```

### Generated Music Prompt

```text
Introduce a new musical theme: Add
Reuse an existing musical theme: Add
Create a tense branching progression
Add a feeling of resolution
```

---

## Repository Structure

```text
.
├── main.cpp        # Clang AST analyzer
├── main.py         # Music prompt generator
├── Program.cpp     # Example input program
├── events.json     # Generated event data
├── CMakeLists.txt
```

---

## Requirements

### C++

- C++20
- LLVM
- Clang LibTooling
- nlohmann/json

### Python

- Python 3.10+

---

## Build

### macOS (Homebrew Example)

Install dependencies:

```bash
brew install llvm nlohmann-json
```

Configure and build:

```bash
cmake -B build
cmake --build build
```

---

## Usage

### 1. Analyze C++ Source

```bash
./build/Cpp2MusicPrompt Program.cpp -- -std=c++20
```

This generates:

```text
events.json
```

### 2. Generate Music Prompt

```bash
python3 main.py
```

Example output:

```text
===== GENERATED PROMPT =====

Please compose music that represents the following program structure.

Introduce a new musical theme: main
Add a short musical motif: result
Reuse an existing musical theme: Add
Create a tense branching progression
Add a feeling of resolution
```

---

## Technical Details

The analyzer uses:

- `RecursiveASTVisitor`
- `FunctionDecl`
- `VarDecl`
- `IfStmt`
- `ForStmt`
- `WhileStmt`
- `DoStmt`
- `CallExpr`
- `ReturnStmt`
- `CompoundStmt`

to traverse the Clang AST and serialize events into JSON.

Events are sorted by source location and priority before export.

---

## Future Ideas

- MIDI generation
- Direct DAW integration
- Harmony generation from control flow
- Rhythm generation from loops
- Dynamic orchestration from nesting depth
- Real-time visualization
- Support for additional languages
- AI-generated soundtrack integration

---

## Motivation

This project is not intended to create "correct" music.

Instead, it explores:

> "What if program structure itself could become a form of musical notation?"

---

## License

MIT License
