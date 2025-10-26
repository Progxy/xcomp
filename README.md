# 🗜️ XComp — A Unified Multi-Format Compression Library

**XComp** is a modular encoder/decoder framework that provides a unified interface over multiple compression formats (e.g., **zlib**, **zstd**, and more).
It allows developers to **compress and decompress data** using a consistent API, either via per-format headers or through a single high-level interface.

---

## 🚀 Features

* 🔹 **Multi-format support** — currently includes `zlib` and `zstd`, easily extensible.
* 🔹 **Single-header build** — use `xcomp_lib.h` for lightweight integration with dynamic or static library.
* 🔹 **Amalgamate Header** — use `xcomp.h` to access all formats through a common function set.
* 🔹 **Flexible linking** — build as static (`.a`), dynamic (`.so`/`.dll`), or header-only.
* 🔹 **Recursive testing** — each module has independent tests, executed from the root `Makefile`.

---

## 🧩 Project Structure

```
xcomp/
├── Makefile              # Root makefile (builds all modules, runs tests, and assembles library)
├── zlib/
│   ├── Makefile          # Module makefile (builds zlib encoder/decoder)
│   ├── xcomp_zlib.h      # Header-only version for zlib
│   └── ...               # Source files and tests
├── zstd/
│   ├── Makefile
│   ├── xcomp_zstd.h
│   └── ...
└── build/
    ├── xcomp.h           # Amalgamate header
    ├── xcomp_lib.a       # Static library
    ├── xcomp_lib.so      # Dynamic library
    └── xcomp_lib.h       # Header exposing simple inflate/deflate functions for use with dynamic/static library
```

---

## ⚙️ Building

From the project root, you can use the provided **Makefile** to build, test, or export the combined library.

### 🧪 Run all tests

```bash
make test
```

This will recursively invoke each subdirectory’s `Makefile` to test every compression format.

### 📦 Generate Amalgamate Header Library

```bash
make amalgamate
```

This produces `build/xcomp.h`, combining all module headers into one portable file.

### 🔗 Build static/shared libraries

```bash
make static   # -> build/xcomp_lib.a
make shared   # -> build/xcomp_lib.so (or .dll on Windows)
```

---

## 🧠 Usage

Include the unified header and call the generic compression API:

```c
#include "xcomp.h"

int main() {
    const char *input = "Hello, XComp!";
    size_t input_size = strlen(input);

    unsigned char compressed[1024];
    unsigned char decompressed[1024];
    size_t compressed_size, decompressed_size;

    // Compress using Zlib format
    xcomp_compress(compressed, &compressed_size, input, input_size, XCOMP_ZLIB);

    // Decompress back
    xcomp_decompress(decompressed, &decompressed_size, compressed, compressed_size, XCOMP_ZLIB);

    printf("Result: %s\n", decompressed);
    return 0;
}
```

---

## 🧩 Supported Formats

| Format        | Header             | Module Folder | Status         |
| :------------ | :----------------- | :------------ | :------------- |
| **zlib**      | `xcomp_zlib.h`     | `zlib/`       | ✅ Stable       |
| **zstd**      | `xcomp_zstd.h`     | `zstd/`       | 🚧 In progress |
| **(more...)** | `xcomp_<format>.h` | `<format>/`   | 🔜 Planned     |

---

## 🧾 License

This project is licensed under the GPL3 **License** — see [`LICENSE`](LICENSE) for details.

---

## 🌟 Future Plans

* Add support for LZMA, Brotli, and Bzip2.
* Provide Python/C++ bindings for scripting and native integration.
* Add benchmarking and comparison tools.
