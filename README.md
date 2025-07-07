
# 🎬 Subtitle Merger / Metadata Injector / Embed STR

A simple CLI tool to **attach subtitles** and optionally **inject metadata** into video files using `FFmpeg`.  
Available in two versions: **Python** (lightweight, no dependencies) and **C++** (compiled, faster execution).

---

## 📦 Features

- 📝 Add or replace subtitle tracks
- 🔤 Set subtitle language (e.g: `eng`, `fa`, `farsi`)
- 🎯 Inject metadata into video, audio, and subtitle streams
- 🧹 Optionally remove all existing subtitle tracks before attaching new ones
- ⚡ Fast C++ version using `nlohmann/json` and FFmpeg tools
- 🐍 Python version with no external dependencies

---

## 🧰 Requirements

- For CPP
  - [FFmpeg](https://ffmpeg.org/download.html) and [FFprobe](https://ffmpeg.org/ffprobe.html) (must be in your system `PATH`) - [FFmpeg static build for windows](https://johnvansickle.com/ffmpeg/)
  - C++ version only:
    - CMake 3.30+
    - C++20 compiler
    - [`vcpkg`](https://vcpkg.io) for managing dependencies (used here for `nlohmann/json`)
- For Python
  - Python installed (v3)

---

## 📂 Project Structure

```
subMerge/
├── cpp/
│   ├── CMakeLists.txt
│   ├── main.cpp
├── python/
│   ├── main.py
├── LICENSE
├── README.md
```

---

## 🚀 How to Use

### 🐍 Python Version

#### Run directly:
```bash
python python/main.py -i path/input.mp4 -s path/subtitle.srt -o path/output.mp4 -l fa --clear-subs -m ro-ox.com
```

#### Arguments:

| Option           | Description                                                      |
|------------------|------------------------------------------------------------------|
| `-i`, `--input`  | Input video file (required)                                      |
| `-s`, `--subtitle` | Subtitle file to attach (e.g: `.srt`)                            |
| `-o`, `--output` | Output file (optional — defaults to overwriting the input)       |
| `-l`, `--lang`   | Language for the subtitle (default: `English`)                   |
| `-m`, `--metadata` | Add metadata (`title=ro-ox.com`) to video/audio/subtitle streams |
| `--clear-subs`   | Remove all existing subtitle tracks before attaching the new one |

---

### ⚙️ C++ Version

#### ✅ Build Instructions

> Requires `vcpkg` for `nlohmann_json`

1. Clone the repo and initialize submodules (if any):
```bash
git clone https://github.com/amirroox/subMerge.git
cd subMerge/cpp
```

2. Install `nlohmann-json` using `vcpkg`:
```bash
vcpkg install nlohmann-json
```

3. Configure and build:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

4. Run:
```bash
./build/subMerge -i path/input.mp4 -s path/subtitle.srt -o path/output.mp4 -l eng --clear-subs -m ro-ox.com
```

#### Arguments

Same as Python version — full parity of features.

---

## 💬 Example

```bash
# Attach subtitle and clear existing ones (Other file - with `-o`)
python python/main.py -i movie.mp4 -s subtitle.srt -o movie_with_subs.mp4 -l fa --clear-subs

# Attach subtitle and add custom metadata (The same file - without `-o`)
./build/subMerge -i movie.mp4 -s subtitle.srt -m CustomTitle -l eng
```

---

## 📝 Metadata Behavior

When `-m` or `--metadata` is passed without a value, the following metadata will be injected:
```bash
-title         ro-ox.com
-metadata:s:v  title=ro-ox.com
-metadata:s:a  title=ro-ox.com
-metadata:s:s  title=ro-ox.com
```

---

## ⚖️ License

MIT License © 2025 [AmirRoox](https://github.com/amirroox)

---

## 🤝 Contributing

Feel free to fork and open PRs. Suggestions and improvements are welcome!

---

## 📌 Notes

- Output will overwrite the input if `-o` is not specified.
- `ffmpeg` and `ffprobe` must be available in your system's `PATH`.

---
