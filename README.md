# voxcii

Terminal-based ASCII 3D model viewer written in C++

---

## Features

* Real-time 3D rendering in the terminal
* ASCII shading based on surface lighting
* Z-buffer for correct depth and occlusion
* Supports **OBJ** and **STL** models
* Polygon triangulation for complex OBJ faces
* Optional material colors (when supported)
* Automatic rotation or interactive mode

---

## Build

### Requirements

* C++17 compatible compiler
* `ncurses`

### Compile

```
g++ -std=c++17 -O3 -Wall src/*.cpp -o voxcii -lncurses
```

Or with Makefile:

```
make
```

---

## Usage

```
./voxcii [options] model.obj
```

### Options

| Flag                   | Description                 |
| ---------------------- | --------------------------- |
| `-i`, `--interactive`  | Enable manual rotation      |
| `-c`, `--color`        | Enable colored rendering    |
| `-z`, `--zoom <value>` | Initial zoom (default: 100) |

### Controls

| Key        | Action                         |
| ---------- | ------------------------------ |
| `+`/`-`    | Zoom in/out                    |
| Arrow keys | Rotate (interactive mode)      |
| `q`        | Quit                           |

### Supported Formats

* `.obj` (with optional `.mtl` material colors)
* `.stl` (ASCII and binary)

---

## Notes

* Output quality depends on terminal size and font
* Color support depends on terminal + ncurses capabilities
* OBJ material colors require the `.mtl` file to be present
