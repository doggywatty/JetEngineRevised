# JetEditor: scope

Scoping only. No source file was changed. Host: Linux, root disk 96% used (1.6 GB free of 252 GB). Date 2026-10-09.

## 0. Summary

- The editor has no code yet. The renderer, model/level loaders and RecordDB code are real and largely reusable.
- Formats: a pig reader exists and decodes all 387 models; the only writer is RecordDB binary. Nothing writes pig, bclara entities or TGA/PVR.
- Renderer is GLES 2.0 on Linux/Android (EGL), legacy GL 2.1 on macOS (CGL with no profile), and no Windows display at all.
- No UI toolkit is vendored. Recommended: Dear ImGui (docking), one new dependency.
- Eight milestones (section 6), formats first.

## 1. Existing asset formats

Sizes come from `find`/`wc` in this checkout. `data/` is gitignored (`.gitignore`: `/data/`); its tree is 728 MB.

**pig models** (`data/models/*.pig`, 387 files, 27.5 MB, median 26.7 KB, largest 1.19 MB)
- Reader: `core/jet/src/scene/modelbase.cpp` (9.2 KB; `LoadV100` line 64, node 126, camera 152, multires 168), `core/jet/src/scene/modelloader.cpp` (3.6 KB, cache by name), `core/jet/src/scene/submesh.cpp` (15.5 KB, vertex blob decode in `LoadAttributes`), `core/jet/src/scene/mesh.cpp`, `node.cpp`, `camera.cpp`.
- Decoder: `re/py/pig.py`. Run this session over all 387 files: 387 parse, 0 leftover bytes.
- Spec: `re/NOTES.md`, section ".pig (ModelBase::LoadV100 @0x701a70)".
- Writer: none. Runtime `MapAttributeU8Write` in `submesh.cpp` writes GPU buffers only.
- Unknown: meaning of the submesh `u0c` attribute mask and `u44`/`u48` fields (the decoder names them by offset only), the `a0` byte after the nodes, node `flags` bits beyond "camera", the trailing `#source_path` table, and whether empty texture slots matter. Round-trip tests would prove reading only, not fidelity.

**Clara binary database** (`data/levels/*.bclara`: `level.bclara`, `level_empty.bclara`, `intro.bclara`; plus `*.blibclara`: `designlib`, `lablib`, `macholib`; 112 KB for the three .bclara)
- Readers: `core/claraloader/src/` (63 KB total): `project.cpp` (10.4 KB; reads `u32 version`, counts, then strings), `clara.cpp`, `entity.cpp` (13.7 KB), `param.cpp` (9.4 KB), `type.cpp`, `template.cpp`, `folder.cpp`, `group.cpp`, `movie.cpp`, `multilayer.cpp`. `recorddb.cpp` (16.6 KB) reads the RecordDB binary used by settings and savegame.
- Dev tools (compiled this session into the scratch directory against the existing `build/*.a`, not into a new build dir):
  - `tools/dev/claradump.cpp`: runs. `level.bclara` gives 35 top-level entity records with their parameters (`NODE_REF`, `SCRIPT`, `ENTITY`, `ENUM`, `SCALAR` value types, as in the tool's `Value()`).
  - `tools/dev/rdbdump.cpp`: runs. `data/settings` loads ("11 records, format 1"). The 0.0.3 `savegame` is not in `data/`.
  - `tools/dev/redx.cpp`: runs, but writes 0 bytes for `level.bclara`, so that file is not wrapped in a RedundantStream. The wrapper is for other files.
- Writer: `Record::SaveBinary` (`recorddb.cpp:62`) and `RecordDB::SaveXML` (`:426`) exist. Nothing writes `Project`, `Entity` or `Param`.
- Unknown: the leading `u32` and count fields of the bclara header (the loader reads them but the file's magic is unnamed), the template grammar needed to write new entities, and the authoring source. Whether a text form exists anywhere is unknown.

**TGA textures** (`data/textures_android/`: 400 `*.tga` and 73 `*.tga__alpha`, 473 entries, 51.0 MB)
- Despite the extension, these are PVR v3 containers ("PVR\x03", pixel format 6 = ETC1, per `re/NOTES.md`).
- Reader: `core/jet/src/video/gles20/gles20texture.cpp` (PVR3 header struct line 37, detection line 260, ETC1-to-RGB software decode `DecodeETC1` line 180, uncompressed TGA detection line 262).
- `core/jet/src/video/gles20/pvrtdecompress.cpp` is 48 bytes (stub). `core/jet/src/video/soil/stb_image_aug.cpp` is 45 bytes, so stb_image is not vendored.
- Writer: none.
- Unknown: how the `__alpha` companion files are laid out (separate ETC1 alpha plane, per the notes), mip counts, and whether any file is plain TGA.

**XML shaders** (`data/shaders/`: 101 files, 92 `.xml`, plus `.vs`, `.fs`, `.vsh`, `.pass`, `_inc_*.fs/.vs`)
- Readers: `core/jet/src/video/rendertechnique.cpp:108` (pugixml `load_buffer`), `rendertechniqueloader.cpp` (4.5 KB), `gles20shaderprogram.cpp` (version pragma at line 22, `#include` expansion in-engine).
- Each pass declares `target="GLES2.0"` (99 tags in 92 files). `GLES20Driver::GetArchitectureString` returns "GLES2.0" (`gles20driver.cpp:41`), and `rendertechnique.cpp:78` uses it to select passes. A GLES3 backend must change this selection.
- Writer: none.
- Unknown: whether the `.vs`/`.fs` files are the source or are generated from the XML.

**Level, XML, Lua and other content**
- Level sequences: `data/levels/levelsequence2_labo.graphml.xgraphml` (1.7 MB) and `levelsequence2_macho.graphml.xgraphml` (0.98 MB), read by `source/game/leveltemplate/LevelSequenceGraphParser.cpp`. Two `*.texturestreaming` files (4.7 KB each), reader unknown.
- Data libraries: `data/datalibs/` (10 files: tilemaps, states, sprites, gui_pal_list).
- JSON: `data/localizationText/` (4), `profiles.json`, `tracker.json`, `eventparameters.json` (64 KB).
- Lua: `framework/src/luavm.cpp` exists, but `data/` holds no `.lua` files.
- Other: `data/anims` (438 `.anim`, `core/jet/src/anim/`), `data/ps` (169 `.ps`, `core/particles`), `data/collisions` (54 `.coll`, `framework/include/physics/collisionmesh.h`), `data/sounds` (296 `.mpc`, 14 `.vxn`, 9 `.wav`), `data/dlcs` (5 `.jpk`, read as zip by `core/jet/src/stream/zipstreamfactory.cpp`).
- No FBX file exists in the repo or on this host.

**Game binary and packs**
- `data/core` is a 546 MB x86-64 ELF core dump, not the game. The 0.0.3 ARM library is `lib/armeabi-v7a/libdespicablemefree.so` (13.7 MB), inside `.minion rush stuff for reversal/*.apk`. The APK also holds `assets/data_pack1..4.7z`.
- `tools/armdis.py data/core -s SubMesh` returns nothing, because the file is not ARM.
- `py7zr` is installed, so the 7z packs can be read from Python. No `7z` binary is present.

## 2. Tools in the repo

| Path | What it does | Runs here |
|---|---|---|
| `tools/dev/claradump.cpp` | dump Clara entities and params | yes, after compiling into scratch |
| `tools/dev/rdbdump.cpp` | dump a RecordDB file | yes |
| `tools/dev/redx.cpp` | unwrap a RedundantStream | yes (no output for bclara) |
| `tools/dev/README.md` | notes for the three above | n/a |
| `tools/armdis.py`, `armxref.py`, `armcall.py` | ARM disassembly, xref index, unicorn calls | need the extracted `.so`; `armcall` needs `unicorn` (missing); `armxref build` would write into `data/` |
| `tools/propagate.py`, `vtnames.py` | vtable and symbol naming for MSVC binaries | need `pefile` (missing) and a DLL and PDB dump, neither in repo |
| `tools/fillheaders.py`, `linematch.py`, `pdblines.py` | PDB layouts and line tags | need `llvm-pdbutil` dumps, not in repo |
| `tools/buildall.sh` | mac, android, windows, ios builds | no: zsh, `xcrun`, NDK under `~/Library` |

Present here: `llvm-pdbutil`, the mingw-w64 g++, and `capstone`.

## 3. Renderer

Files (sizes in bytes):
- `core/jet/src/video/gles20/`: `gles20driver.cpp` 36.9 KB, `gles20interface.cpp` 41.5 KB (GL loader and wrappers, about 150 GL names used), `gles20shaderprogram.cpp` 22.4 KB, `gles20texture.cpp` 19.5 KB, `gles20geometry.cpp` 17.4 KB, `gles20rendertarget.cpp` 4.4 KB, `gles20rendertargetwrapper.cpp` 7.1 KB, `gles20shaderprogramflavor.cpp` 2.1 KB.
- `core/jet/src/video/egl/jetegldisplay.cpp` 7.8 KB: `EGL_OPENGL_ES2_BIT`, `EGL_CONTEXT_CLIENT_VERSION 2` (lines 51, 87, 125), and a Mesa surfaceless fallback for headless runs (line 37 onward).
- `core/jet/src/video/mac/macdisplay.cpp` 4.0 KB: CGL pixel format with no `kCGLPFAOpenGLProfile`, so the context is legacy GL 2.1. Shaders are `#version 120` on Mac (`gles20shaderprogram.cpp:22`).
- `core/jet/src/platform/win32/wgldisplay.cpp` is 46 bytes (empty). `wgldisplay.h` is 2.5 KB. There is no Win32 window code. The GL loader has a `_WIN32` branch that loads through `wglGetProcAddress` (`gles20interface.cpp` ~lines 93, 245-270).
- `core/jet/src/video/driver.cpp` 8.9 KB and `display.h` (abstract `Display`: `Init`, `Swap`, `CreateSharedContext`, `SetThreadContext`). These are the seams for a new backend.

Current GL version: GLES 2.0 on Linux (Mesa via EGL, verified: `build/test_render` runs headless and prints "render ok"), Android (EGL), and iOS; GL 2.1 legacy on macOS.

GLES2-only calls and assumptions:
- VAOs via `glBindVertexArrayOES`, `glGenVertexArraysOES` (`gles20interface.cpp:289-291`), multisample via `glRenderbufferStorageMultisampleEXT` and `glBlitFramebufferEXT` (`:293-294`).
- Desktop shims for `glClearDepthf`, `glDepthRangef`, `glGetShaderPrecisionFormat` (stubbed), `glShaderBinary`, `glReleaseShaderCompiler` (`:84-88`).
- `iglMapBuffer` (`:561`) assumes `GL_OES_mapbuffer`.
- Shaders use GLSL ES 1.00 idioms: `attribute` (96 files), `varying` (61), `texture2D` (25), `gl_FragColor` (62). The precision header hack is at `gles20shaderprogram.cpp:193`.
- Attribute locations are bound by name (`gles20shaderprogram.cpp:230-235`).
- Texture formats `GL_ALPHA` and `GL_LUMINANCE` (`gles20texture.cpp:53-55`) do not exist in GLES 3 or GL 3.2 core. They need `R8` with a swizzle.
- PVRTC enums (`gles20texture.cpp:10-15`) matter only on iOS. ETC1 is decoded in software on Mac (`:179`).
- Feature detection is by extension substring (`gles20driver.cpp:47-55`).

What a GLES 3 backend needs:
- Core VAOs, `glMapBuffer`, `glBlitFramebuffer`, multisample renderbuffers, instancing (`glDrawArraysInstanced`, `glVertexAttribDivisor`), and optionally UBOs. All are core in GLES 3.0.
- GLSL ES 3.00 (`#version 300 es`) as the baseline. Use a prelude that maps `attribute`/`varying`/`texture2D`/`gl_FragColor` to `in`/`out`/`texture`/a declared `fragColor`. This avoids touching the 99 XML passes beyond the target name.
- ETC2 is mandatory in GLES 3.0. ETC1 data should decode through the ETC2 RGB8 path (believed; verify on a device). The software ETC1 decoder stays for Mac.
- sRGB (`GL_SRGB8_ALPHA8`) is core. Float render targets need `EXT_color_buffer_float` on GLES 3.0.
- GLES 3.2 is best-effort: detect it at runtime. Geometry and tessellation are not needed by this engine.

Feasibility:
- **Android, iOS**: GLES 3.0 is the floor. iOS tops out at ES 3.0 (Metal is the path beyond it). Android 3.2 is present on newer GPUs only.
- **Linux (Mesa)**: GLES 3.x can be checked with the existing `test_render` and `test_shaders`.
- **macOS**: GL 3.3 core (`#version 330`) is the best fit. It is the highest version that gives instancing in the core profile and is available on the Mac. Set `kCGLPFAOpenGLProfile` to the 3.2 or 3.3 core value. Feasible, medium.
- **Windows**: two options. (a) ANGLE gives GLES 3.0 on D3D11, and ES 3.1 with feature level 11_0 or higher. ES 3.2 needs ANGLE's Vulkan backend (verify against current ANGLE docs). Costs a new binary dependency. (b) WGL 3.3 core via `wglCreateContextAttribsARB` (`core/jet/include/platform/win32/wglext.h` is 150 bytes, so the extension header must be written). Feasible, medium. Option (b) needs no new dependency beyond a generated GL loader.

## 4. UI

Vendored under `libs/`: third-party packages are README-only (`boost`, `curl`, `openssl`, `lzma`, `logog`, `tinythread`, `ustl`, `protocolbuffers`, `libs/*/README.md`). Real code: `freetype2` 3.4 MB, `bulletphysics` 3.0 MB, `zlib` 1.1 MB, `lua` 0.43 MB, `vox1.1` (musepack) 0.47 MB, `json` 0.23 MB, `pugixml10` 0.28 MB. Gameloft: `glf` 53 KB (mostly headers; its SDL input file is a stub), `glwt2` 51 KB.

No GUI toolkit is vendored: no Dear ImGui, Qt, wx or nanovg. Window code is macOS-only, in tests: `tests/test_render_window.mm` (118 lines, NSWindow + NSOpenGLView), `tests/test_intro_window.mm` (84), `tests/test_menu_window.mm` (64). Platform windowing sits in `core/jet/src/video/egl/jetegldisplay.cpp` (ANativeWindow on Android, pbuffer or surfaceless elsewhere).

**Recommendation: Dear ImGui (docking branch), vendored under `libs/imgui`, drawn by the engine's own GL backend, hosted in the platform window (NSWindow now, Win32 later).**
- Gives panels, docking, tree nodes, tables (for the property grid), and renders to an offscreen FBO on Linux for screenshots. MIT licence. About 1.5 MB of source with docking.
- Not Unity-looking out of the box; needs a style pass. Immediate-mode, so the explorer and property grid need some code. Text can use the vendored FreeType.
- Zero-dependency alternative: reuse Jet's sprite and text GUI (`Interface3D`). Every panel, tree and property grid is then hand-written. Choose this only if "no new dependencies" is strict.

## 5. Build matrix

From `CMakeLists.txt`:
- C++17. `core_jet` links `EGL GLESv2 dl` on UNIX-non-Apple, `OpenGL.framework` on Apple, and nothing GL-related on Windows.
- `core_net` links `ws2_32` on Windows. `lib_vox1_1` links `winmm` on Windows, `AudioToolbox` and `CoreFoundation` on Apple, `dl pthread` on UNIX.
- Boost: `find_package(Boost QUIET)`, else `/opt/homebrew/include`. Vendored Boost is absent (README only). The Linux build uses system Boost 1.83.
- `JET_TESTS` (default OFF) builds `tests/test_*.cpp`. The `.mm` GUI tests build only on Apple.
- `buildall.sh` covers mac, android (NDK under `~/Library/Android`, armeabi-v7a, API 21), windows (mingw, core targets only) and ios (arm64, syntax check on the macOS SDK).

Per platform:
- **macOS**: Xcode CLT, Homebrew Boost, OpenGL and Cocoa frameworks. Cannot build here.
- **Windows**: MSVC or mingw-w64. The mingw compiler exists here. The game lib's mingw build was not re-run in this session, to stay within the no-new-build-dir rule and the disk limit. Win GL display is absent (`wgldisplay.cpp` is empty).
- **Linux**: EGL and GLESv2 from Mesa. The existing `build/` (1.1 GB) includes `test_render`, which passes headless.
- **Android**: NDK is not on this host. Cannot build here.
- **iOS**: cannot build here (no `xcrun`). Syntax check only.

Disk: `build/` 1.1 GB, `data/core` 546 MB (core dump, gitignored), `pybullet-*.whl` 99 MB and `data.zip` 89 MB at the repo root, `.git` 365 MB. The owner should remove the core dump and the wheel before any build work. This session deleted nothing.

## 6. Milestones

Order follows the brief: formats, explorer, viewport, builder, GLES 3. Size: S (days), M (1-2 weeks), L (several weeks).

1. **M1: formats library** (`tools/editor/fmt/`, new). Pig write and read; RecordDB and bclara read and write; TGA/PVR3 header reader; XML shader read.
   - Check: a CLI round-trips all 387 pig files, and each re-read matches vertex counts, bounds and textures. `claradump` output for `level.bclara` is identical after a read-write-read cycle.
   - Size: L (the pig writer and the unnamed bclara header are the risk). Depends: none.
2. **M2: project and importer.** Project manifest, asset index, TGA/PVR import, and 7z pack reader (`py7zr` in a helper script, or a C++ reader).
   - Check: `import data/` indexes 387 pig, 473 TGA entries, 438 anim, 92 XML, 319 sound files.
   - Size: S-M. Depends: M1.
3. **M3: editor shell and project explorer.** Native window (NSWindow with CGL on Mac; offscreen on Linux), ImGui docking, tree explorer, property grid, no viewport.
   - Check: a PNG (glReadPixels, as `test_render` does) shows the `data/` tree and the parameters of a selected bclara entity.
   - Size: M. Depends: M2, the UI decision (section 4).
4. **M4: viewport and level editor.** Renders a level's entities with the existing GLES 2 path, select, move, delete, save through the RecordDB writer.
   - Check: open `level.bclara`, move an entity, save, and `claradump` shows the new position. The screenshot matches.
   - Size: L. Depends: M1, M3.
5. **M5: Clara builder (FBX to pig and level data).** FBX binary and ASCII parser, mesh, skin and animation export to pig, entity creation in bclara.
   - Check: a test FBX (none exists, so a fixture must be made) converts to pig. `ModelLoader` loads it and M4's viewport renders it.
   - Size: L. Depends: M1, M4, an FBX fixture.
6. **M6: base project exporter and game asset decompile.** Export `data/` and the APK packs into editor form, and back. Readers for `.anim`, `.ps`, `.coll` already exist (`core/jet/src/anim/`, `core/particles/`, `framework/include/physics/`).
   - Check: export then import is lossless for the formats in M1. The existing `test_intro`/`test_menu` load the exported data from `data/`.
   - Size: M. Depends: M1, M2. Can run alongside M4.
7. **M7: GLES 3 backend.** `GLES30` path in `GLES20Driver`'s place: `#version 300 es` (Linux, Android, iOS) and `#version 330` (Mac), a prelude shim for the GLSL idioms, core VAOs and instancing, ETC2, sRGB, `ALPHA`/`LUMINANCE` swizzles, and the pass target lookup changed from "GLES2.0".
   - Check: `test_render` and `test_shaders` pass on Linux Mesa with a GLES 3 context (the tests print the version). `test_render_window` shows the triangle on Mac in GL 3.3 core. Screenshot diff against the GLES 2 path is within tolerance.
   - Size: L. Depends: M3 can run first; the shader prelude does not depend on the editor. Rework of M4 viewport shots may be needed.
8. **M8: Windows and mobile stubs.** Win32 window with WGL 3.3 core or ANGLE (`wgldisplay.cpp` is empty), Android kept on the `ANativeWindow` path, iOS EAGL stub.
   - Check: the mingw target builds and links the editor `.exe`. Android and iOS compile-only.
   - Size: M for Windows, S for the mobile stubs. Depends: M3, M7.

## 7. Biggest unknowns

1. Pig writer fidelity: the meaning of the submesh mask and the unnamed `u44`/`u48`/`a0` fields. Reading is proven; writing is not.
2. Clara header and template grammar, needed to write entities. No writer exists beyond RecordDB, and the authoring source is unknown.
3. No FBX sample or SDK on this host. The FBX parser size is an estimate.
4. The look of Gameloft's 2013 Jet editor is unknown. `shots/` holds 12 game screenshots (480x854), no editor shots.
5. The GLES 3 ETC1 compatibility and the Windows backend choice (WGL or ANGLE) are unverified.
6. Disk: 1.6 GB free. The 546 MB core dump and the 1.1 GB `build/` need a decision before any build work.
7. Boost is not vendored, so the Linux and Mac builds depend on system Boost.
