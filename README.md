<h1 align="center">voxcii</h1>

<p align="center">
Terminal-based ASCII 3D model viewer written in C++
</p>

<p align="center">
<img src="https://github.com/user-attachments/assets/540dbbcf-1fdb-4e7d-a7fb-486366302226" width="70%" />
</p>

---

## Features

* Real-time 3D rendering in the terminal
* ASCII shading based on surface lighting
* Z-buffer for correct depth and occlusion
* Supports **OBJ** and **STL** models
* Polygon triangulation for complex OBJ faces
* Optional material colors (when supported)
* Automatic rotation or interactive mode

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

## Notes

* Output quality depends on terminal size and font
* Color support depends on terminal + ncurses capabilities
* OBJ material colors require the `.mtl` file to be present

## Contributing

PRs and issues are welcome

<br>

<p align="center">
	<img src="https://raw.githubusercontent.com/catppuccin/catppuccin/main/assets/footers/gray0_ctp_on_line.svg?sanitize=true" />
</p>

<p align="center">
        <i><code>&copy 2025-present <a href="https://github.com/ashish0kumar">Ashish Kumar</a></code></i>
</p>

<div align="center">
<a href="https://github.com/ashish0kumar/voxcii/blob/main/LICENSE"><img src="https://img.shields.io/github/license/ashish0kumar/voxcii?style=for-the-badge&color=CBA6F7&logoColor=cdd6f4&labelColor=302D41" alt="LICENSE"></a>&nbsp;&nbsp;
</div>
