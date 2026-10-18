# JetEditor project and asset formats

Milestone M2. Code: `source/editor/project/` (project, importer, SHA-256) and `source/editor/formats/` (pig from M1,
TGA, PVR3). Test: `tests/test_project_import.cpp`. Scope: `re/JETEDITOR_SCOPE.md`, section 6.

## 1. Project folder

```
<root>/
  project.jetproj          manifest (text, section 2)
  assets/database.txt      asset database (text, section 3)
  levels/                  (empty in M2)
  models/                  imported .pig files
  textures/                imported .tga and PVR3 files
  shaders/                 imported .xml shaders
  sounds/                  (empty in M2)
```

`CreateProject` makes every folder and writes an empty database. It refuses a folder that already holds
`project.jetproj`. `OpenProject` requires the manifest, the database and all six folders.

Text files use UTF-8 bytes, LF line ends, and a final LF. Nothing else is accepted: no comments, no CR.

## 2. Manifest: `project.jetproj`

```
format=1
name=<name>
```

- `format` must be `1`. Any other value is refused.
- `name` is 1 to 200 bytes and has no control characters. It may contain `=`.
- Keys are `format` and `name`, each exactly once. Any other key is an error.
- The writer emits exactly the two lines above, so a file written by the editor reads back to the same bytes.

## 3. Asset database: `assets/database.txt`

```
jetdb=1
<path>\t<type>\t<source>\t<hash>\t<size>
```

One line per asset, with five tab-separated fields:

| field | meaning |
|---|---|
| `path` | project-relative path with `/`, for example `models/1lane_path.pig` |
| `type` | `pig`, `tga`, `pvr3` or `shader` |
| `source` | the path the file was imported from, as given to the importer (not copied, not checked later) |
| `hash` | `sha256:` followed by 64 lower-case hex digits, the SHA-256 of the file at `path` |
| `size` | byte count of the file at `path`, decimal |

Path rules (`IsSafeProjectPath`): relative; no empty, `.` or `..` component; no `\`, no `:`, no control characters;
at most 255 bytes; the first component is one of the six folders; at least two components. Paths are unique.

Stored by `SaveDatabase` each time an import succeeds. A failed import does not change the file.

## 4. Importer

`ImportAsset(project, sourcePath)` decides the type from the extension, checks the content, and copies the file
byte for byte into the folder for its type. The destination name is the source's file name.

| source | check | type | folder |
|---|---|---|---|
| `.pig` | `ReadPig`, then `WritePig`; the written bytes must equal the file | `pig` | `models/` |
| `.tga`, uncompressed truecolor 24 or 32 bit | `ReadTga`, then `WriteTga`; the written bytes must equal the file | `tga` | `textures/` |
| `.tga` or `.pvr` whose first four bytes are `PVR\x03` | `ReadPvr3` (header, length) | `pvr3` | `textures/` |
| `.xml` | parses with pugixml; root element is `<technique>` | `shader` | `shaders/` |

Refused, with an error and no change to the project:

- any other extension (for example `.wav`, `.mpc`, `.fs`, `.vs`, `.pass`);
- a `.pvr` that does not start with the PVR3 magic;
- a file that fails its check (truncated pig, RLE or colour-mapped TGA, bytes after the TGA pixels, XML that does
  not parse, XML with another root);
- a source path with control characters, or a file name that cannot be used in a project;
- a destination path already in the database, or a file already on disk at that path. Nothing is replaced.

A `.tga` that carries PVR3 data (the engine's `data/textures_android` files) is recorded as `pvr3`. The content
decides, not the extension.

On success the file is written, the record is appended and the database is saved. If the database save fails, the
record is removed and the copied file is deleted.

`VerifyAssets` re-reads each file and reports `missing:`, `size differs:` or `hash differs:` per entry.

## 5. TGA (`source/editor/formats/tga.h`)

Read and write for uncompressed truecolor, image type 2, depth 24 or 32, no colour map.

Header, 18 bytes, little-endian:

| offset | size | field |
|---|---|---|
| 0 | 1 | ID length (0 to 255) |
| 1 | 1 | colour map type, must be 0 |
| 2 | 1 | image type, must be 2 |
| 3 | 5 | colour map spec (kept as read) |
| 8 | 2 | x origin (kept) |
| 10 | 2 | y origin (kept) |
| 12 | 2 | width, must be > 0 |
| 14 | 2 | height, must be > 0 |
| 16 | 1 | pixel depth, 24 or 32 |
| 17 | 1 | descriptor: bits 0-3 alpha bits (24-bit: 0; 32-bit: 0 or 8), bit 4 must be 0 (left to right), bit 5 top-left origin, bits 6-7 must be 0 |

Then the ID, then `width * height * depth / 8` pixel bytes, B G R [A] per pixel, rows in file order. The file ends
there. Trailing bytes (including the TGA 2.0 footer) are an error.

The writer emits the same fields, with colour map type 0, and refuses anything the reader would refuse.

## 6. PVR3 (`source/editor/formats/pvr3.h`), read only

PVR3 is little-endian: magic `PVR\x03`, then a 52-byte header:

| offset | field |
|---|---|
| 0 | magic `0x03525650` |
| 4 | flags |
| 8 | pixel format, u64 (a high word other than 0 is refused) |
| 16 | colour space |
| 20 | channel type |
| 24 | height |
| 28 | width |
| 32 | depth |
| 36 | surfaces |
| 40 | faces |
| 44 | mip levels |
| 48 | metadata size |

Metadata follows, then the data. The reader accepts pixel format 6 (ETC1) only. A block is 4x4 pixels in 8 bytes.
Data size is `blocks(mip) * 8 * depth * surfaces * faces` summed over mip levels, where a mip has
`max(1, w >> level)` by `max(1, h >> level)` pixels. The file length must equal `52 + metadata + data` exactly.

Checked on `data/textures_android`: `atlas_aperture_df.tga` (64x64, 7 mips) gives 2796 bytes, and
`atlas_bomb_nm.tga` (128x128, 8 mips) gives 10988 bytes. Both equal the file sizes.

**Writing PVR3 is not supported.** There is no writer. The importer copies the source bytes only after `ReadPvr3`
accepts them.

## 7. Limits

- The importer does not convert: a file is accepted or refused, never transcoded.
- Reimporting a changed source is not supported. Remove the file and its database line by hand, then import again.
- No sounds, levels or GLSL source are imported in M2.
- The pig check reuses the M1 reader and writer, so a pig imports only if it round-trips. The M1 test shows that holds
  for all 387 files in `data/models`.
- The SHA-256 is implemented in `source/editor/project/sha256.cpp` and checked against the FIPS 180-4 vectors in the
  test (empty, `abc`, the 56-byte message, and one million `a`).
