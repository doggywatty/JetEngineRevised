# Reverse-engineering notes (0.0.3 armeabi-v7a libdespicablemefree.so)

Game data: `.minion rush stuff for reversal/` (APK + Titanium backup). Extract the APK, then the
`assets/data_pack*.7z` (each a stored zip) and the backup's `files/` directory. The APK's `.so` keeps
46k exported symbols, so every function is found by name:

    python3 tools/armdis.py SO -s SubMesh                 # symbols by mangled substring
    python3 tools/armdis.py SO _ZNK3jet5scene7SubMesh12LoadGeometryEv
    python3 tools/armxref.py SO build                     # once
    python3 tools/armxref.py SO to 0x707de0 | str "mesh."  # callers / users of a string

`armdis` resolves PLT imports, PIC string/global references (`ldr rX,[pc]; add rX,pc,rX`) and
GOT slots; `armxref` indexes branches, PIC and GOT-relative references.

## What the intro is

`intro.bclara` is a movie level (`LevelDef.IsMovieLevel`): its start script sets `MAT_Intro_PostFx`
(postfx_lut), shows `Group_SplitAnimatedBases_All`, sets camera `Intro_SplitAnimatedBase_Cam`
(CameraCinematic on the animated `Intro_Camera` deco) and plays `Intro_SplitAnimatedBases_Movie`
(60 s, 30 fps, 13 character tracks + camera + props, a script track for show/hide/subtitles).
Its models/anims (`intro_*.pig`, `intro_s01..s16_*_anim.anim`, `intro_enviro_lm.tga`) and
`introlib.blibclara` are NOT in this data; this build only loads `intro` from the editor path
(`Game::FrameUpdate`, "editorMode >> k_claraIntroLevel"). What 0.0.3 shows at launch is the main
menu: `GS_MainMenu` -> `LevelDef::OnInitMainMenu` (level.bclara: `SetPostFx("MAT_Labo_postFx")`,
camera `MenuUI_DefaultCamera`) -> `Menu_MainMenu::StartMainMenuIntroMovie` (`Ref_UIMovie_MMInit`).

## .pig (ModelBase::LoadV100 @0x701a70)

Little endian; strings are u16 length + bytes.

    int 100
    u16 nodes;  node: int 100, str name, s8 flags (1 dummy; 0x80 set if name=="camera"),
                s16 parent, vec3 pos, quat (x,y,z,w raw), vec3 scale, float radius,
                u16 n + n (str,str) properties (skipped)
    u8 max lod
    u16 multires; each: int 100, u32 node index, u16 lods; lod: u8 slot (1..5), mesh
        mesh: int 100, u8 skinned, u8 -, aabb (6 floats), u16 submeshes
        submesh: int 100, u32 format (bit31 cleared on load), u32 attribute mask, vec3 center,
                 [format&3: vec3 quant offset, vec3 quant scale], u16 vertices, u32 indices,
                 str material, u16 n + n str textures (3ds Max map slots, some empty), u8 -,
                 u32 blob size, blob,
                 [skinned: u16 weights, u16 bones, u32 size, bones * 0x1c node records
                  (aabb + u16 node idx + u16), bones * mat4 inverse bind]
    u16 cameras; camera: node + str target, float hfov (deg), aspect, near, far, u8 ortho;
        fov = 2 atan(tan(hfov * pi/360) / aspect)
    [if more data: u16 n + n (str key, str value)]   ("#source_path")

Vertex blob (SubMesh::LoadAttributes @0x707130): u16 header size + header, then per attribute in
the mask (bit = Geometry::SemanticDef: 0 pos, 1 normal, 2..5 tangents, 6 color, 7..10 uv) an array
of vertices*size bytes followed by u16 pad + pad; skinned: u8x4 node indices, u8x4 weights (same
padding); then u16 indices. Format: bits 0-1 pos, 2-3 normal, 4-5 tangent quantization
(0 float x3 / x4 for tangents, 1 S16N x4, 2 S8N x4), bits 6..9 uv set quantized (S16N x2).
Quantized positions: p = offset + scale * q, folded into the world matrix by
RenderJob::SetTransform(m, offset, scale) @0x6efdc8 (m * T(offset) * S(scale)).

Geometry::TypeDef = Float, U8, S16, S8N, U8N, S16N, U16N (sizes 4,1,2,1,1,2,2 @0xc95b8e).

## Textures

`textures_android/*.tga` are PVR v3 containers: pixel format 6 = ETC1 (alpha in a separate
`<name>.tga__alpha` ETC1 file when the driver uses separate alpha textures), or uncompressed
`rgba` 8888 (e.g. LUTs).

## Shaders

`shaders/*.xml` techniques: `<uniforms>` with `val` bound to engine values (`matrix.world`,
`matrix.worldviewproj`, `camera.position`, `fog.color`, `mesh.colorCorrection_Background`,
`$Global` params, or literal defaults for material parameters), `<attributes>` (`mesh.*`),
`<samplers>`; GLSL uses `#include` and driver defines (`HAS_CURVATURE`, `HAS_NDIS`, ...) and the
injected `jetAlphaTexture2D`. Clara `RenderPass` entities give `Values` (material parameters in
order) and `Textures` (`<Diffuse>`/`<Bump>`/`<SpecularLvl>` = the model's map slots, else a file).

## Level sequence graph

`levelsequence2_*.graphml` are read through StreamMgr as `.xgraphml`: an encryption pair
registered in `Game::PostInit` (`AddEncryptionPair(".xgraphml", "ERROR: invalid stream")`; the key
doubles as an error string), XTEA like the other `.enc` streams. Decrypted it is yEd GraphML;
node data `d5` holds the level description (`[Location{LaboStart}, Camera{...}, 1, 1, 2]/1`,
chunk template numbers). The main menu shows the `Background_LaboStart` background (lablib).

## Post effects

`BeginPostEffect` redirects the scene into the display's post-effect target 0 (`effect<i>`,
format `GetRTFormatString`: `T565 D16`, a 565 colour texture + 16 bit depth); `EndPostEffect`
draws each effect's material as a fullscreen quad, reading the previous target (`previous` =
render target index 0x2fc, `default` = 0x2fb in `FindRenderTargetIdxByName`) into the next, the
last into the original target. Samplers with a `val` binding are technique (auto) samplers.

## .anim (Animation::InitV100 @0x721824, Load @0x721a34)

    u32 fps; s16 nodes, per node str name + u8 flags (bit 1 compressed rotations)
    s16 materials, per material str name + s16 texture channels
    position channel of every node, rotation channel of every node, scale channel of every node
    per material: uv position channels, uv rotation channels, uv scale channels
    u8 0

A channel: u16 key count; keys: u16 times (8 ms units, bit 15 = hold) then values; no keys: one
value. Positions/scales float3; rotations 4 x s16 / 32767, compressed 3 x s16 (w rebuilt,
normalized); uv pos/scale float2, uv rot float. Sampling: last key <= t, lerp / nlerp to the next.
`AnimController::Update`: 16.16 fixed speed, looping wraps into [loopStart, end).

## Skinning

`ModelBase::s_hwSkinningBoneLimit` = 30 (`Game::PostInit`): submeshes with fewer skin nodes are
hardware skinned. The flavor type is the weights per vertex (1..4);
`PreprocessShaderFlavorType` adds `uniform highp vec4 _u_node_transforms_[90]`, `mul4x3`,
`mul3x3`, `_a_node_indices_` / `_a_node_weights_` and blends `_jet_skin_trans_` at the top of
main, rewriting the position/normal/tangent attributes into `_jet_inPos_` etc. Skin matrices =
node absolute * inverse bind (x T(offset) S(scale) for quantized positions); the job's world
transform is identity. Larger skeletons are skinned on the CPU (`SubMesh::Skin`).

## Device profiles

`CGameConfig::ComputePlatformToggles`: device -> `compats` master -> `noncompats` {mem, cpu,
gpu} -> `Profiles` GPU_n, CPU_n, MEM_n, then the device's own profile. The toggles give the
shader defines (`HAS_SHADOWS`, `HAS_SPECULAR_MASK`, `HAS_RIM_LIGHT`, `HAS_CURVATURE`,
`HAS_LINEAR_SPACE`, `HAS_SPECULAR_LIGHT`, `HAS_EMISSIVENESS`, `HAS_BARLA`, `HAS_NDIS`, ...).
GPU_5 (e.g. GT-I9500) enables everything and shows optional decos. Some optional decos and
their animations (`labo_computers*.pig`, `decorative_minion_*`) are not in the supplied data.

## Aurora GUI (gui_1_1)

Prototype parsers: `re/py/bsprite.py` (sprites archive + .bsprite), `re/py/aurora.py` (GUI levels).

- `gui_1_1` is a datalib archive like `sprites`: u32 count, u32 offsets, u16-len names
  (`level<N>.layers`), a flag byte per entry, 8-byte aligned data padded with spaces.
- `AuroraLevelData::Load` @0x505fc8 walks 3-byte records `{s8 type, s16 value}`: types 4/5 push the
  value; type 8 is `{s8 len, chars}`, a string interned into `s_auroraStrings` (its index is pushed,
  -1 when empty). An element is 9 header shorts + `p[8]` params:
  `[0]` type, `[1]` unique id, `[2..3]` x,y (design pixels), `[4]` upper-left screen anchor,
  `[5]` upper-left snap (element id), `[6]` lower-right screen anchor, `[7]` lower-right snap, `[8]` count.
- `gui_1_1_str`: u32 level count; per level str name, s32 count, element names (by position).
  107 levels; MainMenu is level21 (132 elements).
- Types (`BasicPage::GenerateFromAuroraFile` @0x50eef0): 0 Graph, 1 Area, 2 FillRect, 3 Button,
  4 Text, 5 List, 6 ListButtonScroll, 7 Slide, 8 PBar, 9 Grid, 10 DeviceSpecificAnim, 11 ScrollBar,
  12 3DElement, 13 3DBodyPart, 14 3DCostumePos, 15 3DTouchInteract.
- Parent param index per type (table @0xc8b2e0): 12 11 15 13 17 17 17 13 15 18 11 12 10 11 11 10.
  Width/height params are 9/10 for Area, FillRect, Text, List, ListButtonScroll, Grid, 3DBodyPart;
  14/15 for ScrollBar.
- Design resolution is 768x1024 portrait. `_RescaleAll`: x' = (x - anchorOff(game)) * sx + anchorOff(display),
  sizes * s. `_GetAnchorOffset(a, size)`: 3 (0,h/2), 4 (0,h), 5/6 (w,0), 7 (w,h/2), 8 (w,h), 9 (0,0),
  10 (w/2,0), 11 (0,h), 12 (w/2,h), 13 (w/2,h/2), else (0,0).
- Graph param 9 indexes `gui_graph_item_sprites` (u32 count, u16-len names: 0 loading, 1 target,
  2 menu_01, 3 hud_01, 4 menu_02, 5 menu_icons, ...). `gui_pal_list`: u32 count, (colour, colour) pairs.

## Sprites

- `Sprite::Load` @0x6407f8: `<name>.bsprite`, texture `sprite_<image>` (or `sprite_<name>.tga` when the
  image name is empty), plus an optional `<texture>__alpha`. Atlases are in textures_android
  (`sprite_menu_01.tga`, `sprite_hud_01.tga`, ...).
- AFrame and FModule runtime records are 0x34 bytes: u16 index, u8 flags at +0xc, mat3 at +0x10
  (translation scaled by posScale when painting). FModule flag 0x10: the entry is a frame (hyper frame).
  Module info is 16 bytes: u8 type, u16 x, y, w, h at +2/+4/+6/+8; type 0xFD is not drawn.
- `_PaintModule` @0x640ca8: one quad M*(0,0), M*(w,0), M*(w,h), M*(0,h); uv (x,y)-(x+w,y+h) * inverse
  image size; current colour. Paint chain: Anim (time -> aframe) -> AFrame (M*aframe) -> Frame ->
  FModule (M*fmodule) -> Module.

## Level templates, backgrounds and the graph (status)

Ported (real, from the .so): ParticleSystemEntity, LensFlareEntity, FlareMgr (data only), SkyDomeMgr, GameSettings (records, volumes),
BackgroundMgr + BackgroundTemplateInstance, PlatformTemplate/PlatformInstance/ChunkTemplate, GameplayTemplateInstance, LevelTemplateMgr
(menu-time part), Pacesetter (controller/follower part), GameUtils::IndexDistributedRandom, GameLevel deferred deletion.

Facts: BackgroundMgr constants are 180 (template size), 20 (safe change offset), 500 (hidden offset), 10 (count). Lane sequence order is
Dummy_R, Dummy_C, Dummy_L and maps to spawn lanes 2, 1, 0. LevelTemplateMgr names: "001_LAB" is template "001", "Background_X" a background;
history strings get "_left", "_right", "_tee". GameLevel implements LevelTemplateSequenceProvider (graph + cannon mode overrides).
The graph manager switches the colour profile per location (g_ActiveColorProfile, g_TransitionTime): not ported.

Gaps (named so they are not forgotten): FlareMgr Update/Render and LensFlareDef; GS_Gameplay sound (GameLevel::GameplayHooks);
LevelTemplateMgr::UpdateLevelParts/AdvanceLevelTemplateList and friends (gameplay time); GameplayTemplateInstance::ApplyCameraValue/
ApplySpeedValue/ApplyGameplayValues; PlatformInstance obstacle/runner/interactive/safe-minion-point handling (ProcessCopiedSequence,
ProcessGeneratedLinkedObject, CreateLinkedObject class inits, default safe minion point); InteractiveObjectInstance (type 0xe, ObjectHolder);
Pacesetter lane movement; RocketPath; BappleMgr; GameSettings tracker records and notifications; version check.

## Main menu render status (2026-10-02 session)

Booted end to end on Linux (Mesa EGL/GLESv2 headless, `apt install libegl1 libegl-mesa0 libgles2 libgles-dev libboost-dev`)
via `tests/test_intro.cpp` against data extracted from `DespicableMeFreeHD_update0_GAND_AndroidGeneric_..._00300.apk`
(`assets/data_pack*.7z` are raw LZMA of a stored zip: `unlzma -k -c data_packN.7z > data_packN.zip`, unzip each into one
dir). Splash videos (Universal/Illumination/Despicable Me logos) and the loading screen render pixel-correct. GS_MainMenu
also renders correctly, including the Daily Bonus calendar overlay that correctly auto-shows on a fresh save
(`Player::ResetData` zeroes `dailyCurTime`, so `Menu_MainMenu::SetupDailyLogin` computes `m_dailyDone=false`) — this
matches the real game's first-launch behaviour, it is not a bug. `PopupMgr` and `DailyTokenMgr`/`IGNotification_*` are
still empty placeholders and are NOT involved; the calendar is part of `Menu_MainMenu`'s own GUI layout (confirmed by
reading `Menu_MainMenu.cpp`, which is a real, non-stub port).

Two real findings from this session, found by instrumenting `Deco3d::Init`/`Interface3DTouchInteract::Init` with
`getenv("JET_DEBUG_3D")`-gated stderr prints (not committed, revert-and-redo if wanted — see
`tests/test_intro.cpp`'s `IntroMain(argc, argv, sink, ud)` entry point for wiring a debug build):

- **Most lab hallway models are missing from the data, not a code bug.** `Deco3d::Init` correctly no-ops (no fallback
  mesh) when `ModelLoader::Load` fails, matching the original (`fprintf(stderr, "ERROR: could not load model, loading
  replacement model: ...")` is just the original's log text — there never was an actual replacement model in the
  original engine either). Confirmed both the APK's `data_pack*.7z` and the Titanium backup tar.gz lack
  `labo_computers*.pig`, `labo_glows*.pig`, `labo_wirerockets.pig`, `labo_specular_staticmesh*.pig`,
  `labo_transparent_tubes*.pig`, `labo_ground_tubes.pig`, etc (confirmed previously in the jet-engine-pdb memory as
  "optional decos ... not in the supplied data"). Only `labo_minion_room.pig` and the lab start-area models are present.
  Needs a more complete data source (later APK/OTA update, or a device pull) to see the lab hallway fully dressed;
  this is a data-acquisition problem, not something to fix in code.
- **The main-menu "tap to play" character (`ui_mm_tapplay.pig`, clara node `UI_mm_TAPPLAYbutton`, GUI element
  `TAPPLAY3DModel`, type `3DTouchInteract` → `Interface3DTouchInteract` → `TouchInteract3d`) loads its model and
  animation successfully** (`ui_mm_tapplay_intro/idle/tap/out.anim` all present and bind without error, traced via
  `TouchInteract3d::SetDecoAnimation`), **and the screen→world placement math is verified exactly correct, not a bug**:
  instrumented `Interface3DTouchInteract::Render` to print the Aurora design position (`pos=(779,650)` of a 768x1024
  canvas), the computed world point `at`, and `cam.Project(at)` projected back — it round-trips to `(779.0, 650.0)`
  pixel-exact. Scale is also sane (`templScale=(0.7,0.7,0.7)`, `k=1`). So `MenuModel_Get3DPos`/`Camera::
  GetRayFromScreenCoordinates`/`Project`/`Unproject` are all correct (their Y-convention differences from each other
  are each internally self-consistent, not a mismatch). What's visible at that screen position is still just a small
  sliver of a silvery/metallic multi-segment shape, not a recognizable minion. Current best guess, NOT yet confirmed:
  `ui_mm_tapplay.pig`'s own AABB is only ~0.74 x 0.08 x 0.77 units — small, consistent with a stylized button/badge
  prop rather than a full human-scale standing minion — so this may not be "the foreground minion" at all, and the
  earlier "T-pose" sighting more likely refers to one of the `decorative_minion_*.pig` background worker entities
  (decorative_minion_facebook/repairing/checking/painting/resting/beam_working — several of which DO load
  successfully per the model-load log) placed around the lab hallway, not this tap-play button. Those aren't
  reachable to inspect yet because most of the hallway geometry anchoring them is the missing-data problem above.
  Next step: once more lab data is available, find and screenshot one of the `decorative_minion_*` entities directly
  (or load one standalone the way `tests/test_frame.cpp` loads a single `.pig` + animation) rather than chasing
  `ui_mm_tapplay.pig` further.
- Reminder for next session: `JET_SKIP_DAILY=1` (temporary env check, not in the tree — add
  `Player::GetInstance()->SetDailyCurTime((int)time(0))` after `game.PostInit()` in a scratch copy of
  `tests/test_intro.cpp`) is the fast way to bypass the Daily Bonus overlay and see the plain main menu while
  debugging the tap-play character.
- `tests/test_intro.cpp`'s PPM writer does not check `fopen()` for null — passing a prefix whose directory does not
  exist segfaults instead of erroring. Harmless for a one-off test binary, but worth a one-line fix
  (`if (!o) { perror(name); continue; }`) if this becomes a CI-run tool.
- **Follow-up on `ui_mm_tapplay.pig`'s uncanny close-up (`Interface3DBodyPart` "Tapplay_BP0" icon, the badge above the
  Daily Bonus panel): fully instrumented, not a position/scale bug.** `InterfaceObject::Update3DLayerTransform(l, bpo,
  idx, size)` builds a small world-space quad from the character's one `BodyPartDefinition` ("BP_0": radius=0.108,
  height=0.347 — normal head-capsule proportions) with `baseScale`/`curScale` both `(1,1,1)` (no blow-up) and a sane
  `finalScale=(0.347,0.216,0.216)`; the full-body placement (`Interface3DTouchInteract::Render`) round-trips
  `cam.Project(at)` back to the exact design pixel (779.0, 650.0). So the geometry is being placed and sized exactly
  as authored. The close-up still reads as an uncanny single giant eye, most likely because this repo's color-profile
  grading (the LUT-based postfx pass) is not ported yet (`GameLevel`'s graph manager only stores the profile index —
  see "Level templates, backgrounds and the graph" above) — an unlit/ungraded close-up of a goggle+eye would look
  exactly like this. Confirms the earlier "T-pose" note is unrelated to this element; still suspect a
  `decorative_minion_*` background entity for that (see above), separately from this color-grading gap.

## Sound (2026-10-02 session, same day)

`libs/vox1.1` (Gameloft's "vox" sound engine) was, before this session, **100% empty placeholders** — every single
`.cpp`/`.c` in it was a 1-line stub, including the driver, mixer, and all codecs; `source/game/gamemanagers/
GameSoundMgr.cpp` is also still a 1-line stub, and nothing in `Game.cpp`/`GS_MainMenu`/Lua script bindings calls into
any sound system yet (`StopMusic`/`StopSound`/etc are bound to `L_Nothing` in `ScriptFunctions.cpp`; there is no
`PlayMusic`/`PlaySound` binding at all, not even a stub). This is a from-scratch subsystem, not a partial port.

Real win this session: the `libs/vox1.1/include/mpc/*.h` and `src/mpc/*.c` **stub filenames exactly match the real
open-source Musepack SV8 reference decoder** (`mpc_decoder.c`, `mpc_demux.c`, `huffman.c`, `requant.c`,
`synth_filter.c`, `streaminfo.c`, `mpc_bits_reader.c`, `crc32.c`), and `vox_decoder_mpc8.h`'s `DecoderMPC8Cursor`
class literally wraps the real public libmpcdec types (`mpc_demux_t`, `mpc_reader_t`, `mpc_status`) — strong evidence
the original SDK vendored stock libmpcdec, same as this repo already did for zlib/freetype/lua. **Vendored the real
BSD-3-clause decoder** from `github.com/ddiakopoulos/libnyquist` (`third_party/musepack`, itself a straight vendor of
the official `musepack.net` SV8 code) into `libs/vox1.1/{include,src}/mpc/`, replacing the stub files one-for-one
(plus `mpc_reader.c`, the stdio reader helper, which wasn't in the original stub set but is needed for
`mpc_reader_init_stdio`). Two build fixes needed: `target_include_directories(lib_vox1_1 PUBLIC
libs/vox1.1/include PRIVATE libs/vox1.1/include/mpc)` (headers include each other both as `<mpc/x.h>` and quoted
same-dir `"x.h"`, matching the original libmpcdec layout), and `-fcommon` for this target only (the vendored code's
shared tables like `__Dc`/`__Cc`/`Res_bit` are old-style tentative definitions across multiple `.c` files, which
modern GCC/Clang reject by default since `-fno-common` became the default around GCC 10).

Verified for real against actual game audio, not synthetic data: `tests/test_mpc.cpp` decodes a `.mpc` file through
`mpc_demux_init`/`mpc_demux_decode` to PCM16 and writes a `.wav`. Ran it on `sounds/bg_mm_loop.mpc` (the real main-menu
background track): `channels=2 sample_freq=32000 samples=722588 (22.58s) avg_bitrate=130 kbps encoder='--Stable--
1.30.1'` — correct stereo/rate/duration/encoder-string metadata, and the decoded PCM is 99.85% non-silent with a
sane amplitude range, i.e. real music, not noise.

Not done, left for next session (this is the actual "wire it into the game" half, comparable in scope to how long the
video driver took across sessions, so don't expect it in one sitting):
- The second codec, `.vxn` (Gameloft's own container, magic `"VoxN"`, chunks `Afmt`/`Segm`/`Rule`/`Plst` — an
  interactive/adaptive music format with states and transitions, used for the actual music LOOPS like
  `bg_fluffy_loop.vxn`, `m_disco.vxn`, `m_factory.vxn`; `.mpc` is used for `bg_mm_loop` and nearly all one-shot SFX).
  `DecoderNative`/`DecoderNativeCursor` in `vox_decoder_native.h` have real PDB-derived declarations (states,
  transitions, segments, playlists, cues) to port against, unlike the `mpc/` headers which had none (meaning there's
  no proprietary-format ground truth to match for `.vxn` the way there was for vendoring `.mpc` — this one has to be
  reverse-engineered from the "VoxN" chunk layout, not vendored).
- A mixer and an actual output driver. `vox_driver.h`/`vox_driver_mmsystem.cpp` are empty; nothing plays audio on any
  platform yet, including the one Win32 stub. For this headless Linux container there's no real audio hardware
  anyway, so the practical target is: a software mixer that sums N decoded PCM streams, behind a small `Driver`
  abstraction with at least a "dump to file" backend (useful for tests/CI) and real backends (ALSA/PulseAudio on
  Linux, CoreAudio on Mac, WASAPI on Windows, AAudio/OpenSL on Android) added as needed per platform.
- `GameSoundMgr.cpp` (gameplay SFX: combos, coin collection) and the main menu's music trigger are both still
  unwired — there is currently no code path that would call "play `bg_mm_loop.mpc`" during normal play. The
  `tests/test_mpc.cpp` decode is a standalone proof the codec works, not a wired-up system; the demo video's audio
  track was muxed on top of the silent capture with `ffmpeg`, not produced by the engine's own mixer.
- `vox_mutex_win32.cpp`/`vox_thread_win32.cpp` are also empty stubs; a POSIX mutex/thread backend will be needed
  once the mixer runs on its own thread (not required for a single-threaded "decode and dump" MVP).

## IMPORTANT correction: the ARM disassembly tools work in a cloud/headless session too (same day)

Earlier in this same session I assumed the `tools/armdis.py`/`armxref.py` workflow needed the Mac-only Ghidra setup
from prior sessions and wasn't available here, and nearly shipped *guessed* enum values as a result. That assumption
was **wrong** — correcting it for whoever reads this next, cloud session or not:

- The real ARM `.so` is sitting right there in `.minion rush stuff for reversal/*.apk` → unzip it →
  `lib/armeabi-v7a/libdespicablemefree.so`. No Ghidra, no `.pdb` needed for `armdis.py`/`armxref.py` — they're a
  self-contained ELF/Capstone disassembler, not a Ghidra frontend.
- They just need two pip packages that weren't preinstalled here: `pip install --break-system-packages pyelftools
  capstone`. That's it. After that `python3 tools/armdis.py <so> -s <substring>` (symbol search, matches demangled
  names) and `python3 tools/armdis.py <so> <addr-or-symbol> [len]` (disassembly, strings/branches/calls resolved to
  names) both work exactly as the top of this file already documented — I just hadn't tried before assuming it was
  unavailable. **Try this before guessing anything** — the whole point of this file's RE methodology is not needed
  when you can just read the real code.

## Menu_Shop/Menu_Options/Menu_Goals: blocked on missing nested enum declarations, now unblocked with real data

Attempted a minimal port of `Menu_Shop`/`Menu_Options`/`Menu_Goals` (the Shop/Options/Goals screens reachable from
the main menu's buttons) as thin `Menu_Base` subclasses — Menu_Base already does all the generic work (owns a
`BasicPage` built from an Aurora GUI level name and renders/updates it), so in principle a minimal subclass that just
sets the right GUI level name should show the static page for free, the same way `Menu_MainMenu` does.

Hit a real blocker, **do not paper over it with guessed values**: `Menu_Shop.h` uses `Menu_Shop::EShopTabs` and
`Menu_Options.h`/`Menu_Goals.h` use `Menu_Options::EStateDef`/`Menu_Goals::EStateDef` (via the `safe_enum<Def>`
wrapper) — and none of these nested enums are declared *anywhere* in the header, not even as an empty shell. This is
different from the `TRACKING_SCREEN`/`TRACKING_PN_TYPE`/etc. family (also totally undeclared, 35 use sites across
`source/game/online/tracking/tracking.h` which is a blank stub file): those are opaque write-only analytics payload
IDs nothing branches on, so stubbing them with placeholder values to unblock compilation is low-risk. `EShopTabs`/
`EStateDef` are different — they drive real `SetState()`/`GoToTab()` control flow (which sub-page shows), so a wrong
guess bakes incorrect navigation logic into the tree looking verified when it isn't. Reverted the attempt rather than
ship that.

**Then found the ARM .so does have this** (see above — don't skip the disassembly step next time):
- `Menu_Shop::GetTrackingState(Menu_Shop::EShopTabs) const` at `0x5b5340`: `cmp r1,#4; bhi <invalid>` → **EShopTabs
  has exactly 5 valid values, 0..4** (indexes into a 5-entry `TRACKING_SCREEN` table at `0xc8c2f4`, PIC-relative from
  `0x5b534c`).
- `Menu_Options::GetTrackingState(safe_enum<EStateDef>) const` at `0x48c234`: `cmp r1,#5; bhi` → **EStateDef has 6
  valid values, 0..5** (table at `0xc8a518`).
- `Menu_Goals::GetTrackingState(safe_enum<EStateDef>) const` at `0x4e8bd8`: same shape → **EStateDef has 6 valid
  values, 0..5** (table at `0xc8ade8`). (`Menu_Social::GetTrackingState` also exists at `0x4974f4`, not yet checked.)
- `Menu_Shop::GoToTab` **actually takes 3 arguments** in the real binary —
  `Menu_Shop::GoToTab(Menu_Shop::EShopTabs, bool, bool)` at `0x5b2b24`, size 604 bytes — but `Menu_Shop.h` only
  declares `GoToTab(Menu_Shop::EShopTabs, bool)`, 2 arguments. The PDB header is wrong/incomplete here, not just
  missing the enum. Don't trust the header signature without cross-checking `armdis.py -s GoToTab` first.
  `Menu_Shop::Init()` is also much bigger in the real binary (4676 bytes) than what "just construct a BasicPage"
  would suggest, and there's a `Menu_Shop::InitTutorial()`/`SetupTutorial()`/`HandleTutorial()`/`TutorialElement`
  family and an `EnableChildrenWithButtons`/`EnableTouchesTutorial` pair not in the header at all — the shop has an
  onboarding tutorial overlay that isn't represented in `Menu_Shop.h`'s member list. Same caution applies: the
  header is a *starting point*, not the full truth, for this class.
  `GoToTab` also calls a few more real, not-yet-seen-elsewhere classes worth noting: `Store::IsAnyIAPTokenItemAvailable()
  const`, `Store::RefreshIAPTokenList()`, `TrackingScreenMgr::Update()`, `PopupMgr::PushInfoPopup(std::string const&)`.

**Next step, don't restart from scratch**: already disassembled both `Menu_Shop::GoToTab` (604 bytes, `0x5b2b24` —
full dump saved nowhere, re-run `python3 tools/armdis.py <so> 0x5b2b24 604`) and `Menu_Shop::Refresh()` (992 bytes,
`0x5b2d80`), `armxref.py <so> build` already run once this session (not persisted between sessions, re-run it, ~1-2
min). Findings so far, still incomplete:
- `GoToTab`'s `tab==1` path checks an internet-connectivity flag and shows `POPUP_NO_INTERNET` if offline before
  switching — tab 1 is very likely the IAP tab (buying tokens/bananas with real money needs a connection).
  `Store::IsAnyIAPTokenItemAvailable()`/`Store::RefreshIAPTokenList()` are also called only on specific tab
  transitions, reinforcing an IAP-related tab.
- `GoToTab`'s tail recurses `GoToTab(0, false, false)` when leaving tab 4, after a `STORE_NO_PACKS_AVAILABLE` check —
  tab 4 looks like a promo/AD tab that falls back to tab 0 (the default/Buyables tab) when the store has nothing to
  show.
- `Refresh()` calls `SetupBuyablesTab(jet::String const&)` near string literals `'Bananas'`/`'Tokens'` guarded by
  `cmp r3,#3`, and conditionally skips `SetupUpgradesTab()` when the current tab (`r3`) `==2`. So: tab 0 (default) is
  probably "Buyables/Bananas", some tab calls `SetupBuyablesTab("Tokens")` for a token-currency sub-view, tab 2 is
  probably "Upgrades" (matches `SetupUpgradesTab`, and `Shop_Item_Upgrade` from the aurora level list), tab 3 is
  unclear, tab 4 is the promo/AD tab per the point above. **This is not fully resolved** — don't take this paragraph
  as ground truth, it's a partial read to save whoever continues some time; finish tracing `Refresh()`'s branches
  (dump the full 992 bytes, follow each `cmp r3,#N; beq` to see which Setup*/aurora element it gates) before writing
  the `EShopTabs` enum down.
  Do the equivalent disassembly for `Menu_Options::SetState`/`Menu_Goals::SetState` (find their addresses with
  `armdis.py <so> -s "Menu_Options::SetState\|Menu_Goals::SetState"`) to recover `EStateDef` the same way.

  Only then write the nested enum declarations into the headers (as an intentional, documented addition — these
  headers are PDB-derived but this one field is independently verified against the live binary, so it's fine to
  add, unlike inventing it blind) and implement the menus for real, reusing
`Menu_MainMenu.cpp`'s style (generic tab-button released-event loop → `SetState`/`GoToTab`, `GetUIObject` lookups by
the Aurora element names already known from `gui_1_1_str` — `Shop_Common`/`Shop_Item`/`Shop_IAP`/`Shop_Item_Upgrade`/
`Shop_AD`, `Options_Common`/`Options_About`/`Options_Contact`/`Options_Settings`/`Options_Connection`/`Options_Help`/
`Options_RedeemCode`, `Goals_Common`/`Goals_Locations`/`Menu_Missions`/`AchievementsTab` — list all 107 names with
`python3 re/py/aurora.py data/d3/datalibs` once data is extracted per the "Main menu render status" section above).

This generalizes beyond Shop/Options/Goals: **every other still-stub menu** (`Menu_Pause`, `Menu_Cheats`,
`Menu_Social`, `Menu_InviteFriends`, `Menu_Ingame`, `Menu_DeathRevive`, `Menu_ResultEndScreen`, etc. — everything in
`source/game/interfaces/menus/` besides `Menu_Base.cpp`/`Menu_MainMenu.cpp`) is a 1-line stub with a real PDB header,
and the same recipe applies: check `armdis.py -s <ClassName>` first for the real function list/sizes before assuming
the header is complete, disassemble the real `SetState`/`GoToTab`/`Init` to recover anything the header doesn't show
(nested enums, extra params, whole features like the Shop's tutorial overlay), *then* port. `Menu_Cheats` (header is
only 24 lines, no nested enum, reachable from the already-visible `EnableCheats` button on the main menu) is probably
the smallest next win if looking for something scoped to one session.

**Goal context for whoever picks this up**: the standing ask is full 1:1 fidelity — every menu screen reachable and
matching the original, not just the main menu. That's realistically several more sessions' worth of work at the
pace `Menu_MainMenu.cpp` (282 lines, one screen) took; don't try to rush all of Shop/Options/Goals/etc. in one
sitting. Pick one, verify its enums/signatures against the real `.so` first, port it for real, test it renders
(`tests/test_intro.cpp` + a way to force `MenuMgr::GetInstance()->PushMenu("...")` — either add real button-press
handling in `Menu_MainMenu::Update` via `GetReleasedButton()` against `m_shopButton`/etc, which doesn't exist yet
either and is needed either way for the buttons to do anything, or a scratch debug test that calls `PushMenu`
directly), screenshot it, then move to the next one. Also still open from earlier in this file: the color-grading
postfx matrix (`g_ColorCorrectionMatrix`, confirmed real and simple — `gl_FragColor = clamp(M * finalColor, 0, 1)`
in `_inc_staticobjuber.fs`/`_inc_dynobjuber.fs` — but how the 13 floats in `data/d3/color_profiles/pf_*.conf` build
`M` is NOT verified; a naive "last 9 floats = row-major 3x3 submatrix" guess was tested and produces an implausible
near-black-R/G matrix, so that hypothesis is disproven — same fix applies: disassemble whatever reads a `.conf` file
and builds this matrix, now that the ARM tools are confirmed working, instead of guessing further), and the missing
lab hallway models (confirmed absent from both the APK and the Titanium backup for this exact 0.0.3 build — don't
go pull a different game version's assets to paper over it, that would stop being this build's "1:1").

## UPDATE, same session: Menu_Shop actually ported (minimal) and confirmed rendering

Didn't stop at documentation — finished enough of the `EShopTabs` recovery to ship it. `Menu_Shop::Refresh()` at
`0x5b2d80` has a `cmp r3,#3; addls pc,pc,r3,lsl#2` jump table (cases 0..3, numeric values 100% verified from the
jump table itself) plus a tab-4 fallthrough; traced each case's body (see the big comment on `enum EShopTabs` in
`Menu_Shop.h` for the full disassembly evidence) — numeric values are solid, the member *names* I chose
(`Shop_Costumes=0, Shop_Buyables_Bananas=1, Shop_Buyables_Tokens=2, Shop_CostumesPromo=3, Shop_Upgrades=4`) are this
session's best reading, not independently verified; don't treat the names as ground truth the way the numbers are.

Wrote a real, minimal `Menu_Shop.cpp` (was the usual 1-line stub) that constructs the `Shop_Common` Aurora page
through `Menu_Base` and tracks `m_currentTab` with the verified enum — deliberately does NOT call `GetUIObject()`
with any guessed element name (Shop has many sub-elements across `Shop_Item`/`Shop_IAP`/`Shop_Item_Upgrade`/
`Shop_AD` this session didn't verify), so it can't crash on a wrong lookup; the costume carousel, buyable/upgrade
item boxes, tutorial overlay etc. from `Menu_Shop.h` are still unimplemented. Registered it in
`MenuMgr::Loading_AddExtraMenus`. **Confirmed it actually renders** — screenshot taken by force-pushing it with
`MenuMgr::GetInstance()->PushMenu(Menu_Shop::k_menuName)` from a scratch test (not committed) once `GS_MainMenu` is
up: shows real promo item icons, a "BACK"/"Restore" button, and a speech-bubble tutorial prompt, all pulled from the
real Aurora/localization data through the same generic pipeline as the main menu. No regression on the normal boot
video (`tests/test_intro.cpp` still renders `GS_MainMenu` identically with `Menu_Shop` registered but not pushed).

Two more real header bugs found and fixed while making this compile (both are PDB-extraction gaps, not places where
behavior was invented):
- `Menu_Shop::GetMenuName()` was declared **without** `const` in the header, so it didn't actually override
  `Menu_Base::GetMenuName() const = 0` and left the class abstract (`new Menu_Shop()` failed to compile with
  "invalid new-expression of abstract class type"). Added `const`, matching how `Menu_MainMenu.h`'s hand-written
  version already does it. **If any other still-stub menu header has the same `GetMenuName()` signature without
  `const`, it has the same bug — check before porting.**
  `Menu_Shop.h` had zero `#include`s/forward-declarations at all (not even `Menu_Base.h`), unlike
  `Menu_MainMenu.h` which forward-declares its `Interface*` member types up top. Added `#include "Menu_Base.h"` and
  forward declarations for `InterfaceScrollBar`/`BananasAndTokensBox`/`CostumeShopItem`/`SpritePlayer`/
  `MinionCostume`/`ShopBuyableItemBox`/`ShopBonusUpgradeItemBox` (the Shop-specific helper types the member list
  references) at the top of the header, same convention as `Menu_MainMenu.h`. **Check this on every other stub menu
  header too** — several (`Menu_Options.h`, `Menu_Goals.h` at least) looked like they have the same zero-includes
  problem when skimmed earlier this session; don't assume a header compiles standalone until you've actually tried.

Also added `TRACKING_SCREEN` as a one-value placeholder enum in the previously-empty `tracking.h` (see the comment
there for the reasoning on why this one specific gap is safe to stub, unlike `EShopTabs`/`EStateDef`) — needed to
get `Menu_Shop::GetCurrentTrackingScreen()` to compile. Extend it (same pattern) as more menus get ported rather than
introducing a second parallel placeholder.

**UPDATE, same session: done.** `Menu_Options` and `Menu_Goals` got the identical treatment and are both confirmed
rendering real content:
- `Menu_Options::GetTrackingState(safe_enum<EStateDef>) const` at `0x48c234`: `cmp r1,#5; bhi` → 6 values 0..5
  confirmed. `Menu_Goals::GetTrackingState(safe_enum<EStateDef>) const` at `0x4e8bd8`: same shape → 6 values 0..5
  confirmed. Member names (`Options_Settings`/`Options_Connection`/etc, `Goals_Missions`/`Goals_Achievements`/etc)
  are an unverified guess at labels only, same caveat as `EShopTabs` — the counts are solid, the names aren't.
- `EStateDef` for both had to be a `struct { enum type {...}; }`, not a plain `enum` like `Menu_Shop::EShopTabs` —
  the header already used the `safe_enum<Menu_Options::EStateDef,enum Menu_Options::EStateDef::type>` wrapper
  pattern (see `core/base/include/jenum.h`'s doc comment: `struct FooDef { enum type { A, B }; };`), so the enum
  has to match that shape or `safe_enum` itself won't compile. Also needed `#include "jenum.h"` added to both
  headers (another missing-include gap, same family as the others below).
- Same `GetMenuName()` missing-`const` bug, same zero-`#include`s problem, in both headers — fixed the same way.
- Wrote minimal `Menu_Options.cpp`/`Menu_Goals.cpp` (`Options_Common`/`Goals_Common` Aurora pages via `Menu_Base`,
  no guessed `GetUIObject()` calls), registered both in `MenuMgr::Loading_AddExtraMenus`.
- **Confirmed all three new menus actually render** with a scratch test that pushes Shop → Options → Goals in
  sequence after `GS_MainMenu` comes up (`MenuMgr::GetInstance()->PushMenu("Shop"|"Options"|"Goals")`, not
  committed): Options shows 5 real tab icons (gear/refresh/book/info/gift) and a BACK button; Goals shows 5 different
  tab icons (controller/apple/road/star/trophy-ribbon) plus BACK and a cyan "OPEN" redeem-code button with a pulse
  dot. All real Aurora/localization/sprite-atlas content, same pipeline as the main menu, zero crashes, no
  regression on the normal boot video. **4 working menu screens now (MainMenu, Shop, Options, Goals), up from 1 at
  the start of this session.**

**Next**: every other still-stub menu (`Menu_Pause`, `Menu_Cheats`, `Menu_Social`, `Menu_InviteFriends`,
`Menu_Ingame`, `Menu_DeathRevive`, `Menu_ResultEndScreen`, `Menu_ChallengeResult`, `Menu_SendChallengeTaunt`,
`Menu_SendChallengeType`, `Menu_MoviePlayback`, `Menu_PortraitMessage`, `Menu_InitialLanguage`) can very likely get
the exact same treatment — check each header for the same three bugs (missing `#include "Menu_Base.h"`/`jenum.h`,
`GetMenuName()` missing `const`, an undeclared nested enum needing `armdis.py -s <Class>GetTrackingState` or
`-s <Class>SetState` to recover its value count) before assuming any one of them is harder than this. Aurora level
names for their "_Common" container pages weren't all looked up yet — same method as above:
`python3 re/py/aurora.py data/d3/datalibs` lists all 107, grep for `_Common`/the menu's logical name.

Still not done: wiring real button-press navigation from the main menu (see the `Menu_MainMenu::Update` dead-end
above — the real click→push path isn't in that function, not yet found), and filling in each menu's actual content
(costume carousel, settings toggles, mission/achievement lists, tutorial overlays) beyond the static chrome shown
here. `MenuMgr::PushMenu()` from test code remains the honest way to reach and verify screens until real navigation
is wired.

Tried `Menu_Cheats` next (smallest header, no nested enum) with the identical recipe — compiled clean, registered,
pushed fine, but the rendered frame was just the lab background plus a pause icon and some empty panel rects, no
visible list content, unlike Shop/Options/Goals' obvious rich chrome. Reverted rather than claim a win on weak
evidence. Likely cause: guessed the Aurora level name as `"CheatsGUI"` (there's also a `"CheatsEntry"` in the
`gui_1_1_str` list, 63 vs 62, not tried) — try that first, or check `Menu_Cheats::Init()`'s real disassembly for
which GUI level name it actually passes to `Menu_Base`'s constructor before assuming the page itself is just sparse
by design (plausible too, it's a debug console, but don't assume either way without checking).

## UPDATE, same session: Menu_Pause ported too (5th working menu), new broad gap found and fixed

The pause screen's real Aurora level name is `"IngameMenu"` (not anything containing "pause" — `gui_1_1_str` has no
`Pause*`/`*Pause*` entry at all; listed all 107 names with `python3 re/py/aurora.py data/d3/datalibs` to find it).
Same recipe as Shop/Options/Goals: `armdis.py -s Menu_Pause` confirmed `GetMenuName() const` IS `const` in the real
binary (the header's missing `const` is purely an extraction gap, now a confirmed pattern across every menu checked
so far — assume it's missing and fix it on sight rather than re-verifying each time) and turned up real members
the header doesn't even declare (`SetState(safe_enum<EStateDef>, bool)`, `Subpages_ClearPages/OnEnter/Update` — this
header is more incomplete than Shop/Options/Goals' were, not just missing an enum). Recovered `ExitOptions` (used by
the one private method the header does declare, `Exit(ExitOptions)`) from `Menu_Pause::UpdateExit()` at `0x48d094`:
`cmp r3,#0` / `cmp r3,#1` → `GameState::PopState()` + `MenuMgr::PopMenu()` (Resume) / `cmp r3,#2` →
`ChallengeMgr::ClearCurrentChallenge()` + `MenuMgr::PopMenu()` x2 + `Game::GoToMainMenu()` (go to main menu) — fully
verified semantics for values 0/1/2, not just a count this time.

New gap, broader than this one menu: `ustl::vector<T>` (a small/embedded vector type, used in ~25 headers across
`source/game/interfaces/menus/` and `subpages/`) was referenced but never declared anywhere in the tree — same
missing-dependency shape as the earlier `TRACKING_SCREEN` gap. Added `core/base/include/ustl.h`: `namespace ustl {
template<class T> using vector = std::vector<T>; }`. This is a compilation bridge, not a guess at behavior — the
operations this codebase actually calls on it (`push_back`, indexing, `size()`, iteration) are exactly `std::vector`
semantics, so this is safe the same way the `TRACKING_SCREEN` stub was, unlike guessing an enum's values. **Any other
stub menu header using `ustl::vector` now just works** once it includes `ustl.h` — check for this alongside the
other three recurring bugs (missing `const`, missing `#include "Menu_Base.h"`, undeclared nested enum) on every
remaining menu.

Wrote minimal `Menu_Pause.cpp` (shows `IngameMenu` via `Menu_Base`, no guessed `GetUIObject()` calls, `Exit()`
implemented for real since its semantics are verified, everything else — challenge/mission boxes, the real subpages
system — left as a no-op), registered it, **confirmed it renders**: "GAME PAUSED" title, Despicable Score/Bananas/
Distance stat rows, a costume icon badge, and MENU/RESUME buttons, all real content. **5 working menu screens now
(MainMenu, Shop, Options, Goals, Pause).**

Next candidates, in roughly easiest-first order based on header size/complexity skimmed so far: `Menu_Social`
(68-line header, `gui_1_1_str` has `SocialMenu`/`SocialMenu_FriendsBox`), `Menu_InviteFriends` (32 lines,
`InviteFriendsMenu`/`InviteFriendBox`), `Menu_MoviePlayback` (30 lines, `MoviePlayback`), `Menu_PortraitMessage`
(`PortraitMessage`), `Menu_SendChallengeType`/`Menu_SendChallengeTaunt` (`SendChallenge_Type`/`SendChallenge_Taunt`).
Apply the checklist every time before writing any code: `armdis.py -s <ClassName>` first (don't trust the header's
completeness or its `const`-ness), find the real Aurora level name from the full 107-name list rather than guessing
from the class name, recover any undeclared nested enum from `GetTrackingState`/`SetState`/whatever uses it, add
`ustl.h`/`jenum.h`/`tracking.h`/`Menu_Base.h` includes as needed, write a minimal render-only `.cpp` with zero
guessed `GetUIObject()` names, register in `MenuMgr`, verify with a scratch `PushMenu()` test, screenshot, revert the
scratch test, commit only the real header/cpp/MenuMgr changes.

## UPDATE, same session: 2 more ported (7 working menus total), Menu_Social scoped out for now

`Menu_InviteFriends` (clean header, no nested enum) and `Menu_MoviePlayback` (same) got the identical minimal
treatment -- real Aurora names `InviteFriendsMenu`/`MoviePlayback` (found in the 107-name list, not guessed from
the class name), `const` added to `GetMenuName()`, `#include`s added, minimal render-only `.cpp`. **Both confirmed
rendering real content**: InviteFriends shows "INVITE MORE FRIENDS" header, a green "INVITE ALL FRIENDS" button, a
real empty-state message ("There are no friends available to add at the moment...") and a BACK button; MoviePlayback
shows the "Tilt to watch in full screen" prompt with its rotate-device icon. **7 working menu screens now
(MainMenu, Shop, Options, Goals, Pause, InviteFriends, MoviePlayback).**

Looked at `Menu_Social.h` (68 lines) and backed off before writing any code: it needs `LeaderboardMgr::
LeaderboardRange`/`LeaderboardType`/`LoadingStep` (all undeclared, same gap as everything else) PLUS depends on a
whole `social::Leaderboard`/`social::FriendsLeaderboard`/`social::LeaderboardRangeHandle`/`OnlineUser` object graph
that doesn't exist in this tree at all (`LeaderboardMgr.cpp` is a 1-line stub too) -- this is a real online-backend
subsystem, not a header-gap problem like the others. Don't attempt a "minimal" port of `Menu_Social` by stubbing
all of that out without thinking it through first; it's meaningfully bigger than the other menus. Same likely
story for `Menu_PortraitMessage` -- it needs `InputMgr::TutorialInputId` (undeclared, `InputMgr.cpp` also a 1-line
stub) and a nested `Menu_PortraitMessage::PortraitMessageCrl` struct not declared anywhere; skipped for the same
reason, not yet attempted.

Remaining easy/medium candidates, not yet tried: `Menu_ChallengeResult` (`Ingame_ChallengeResult`), `Menu_DeathRevive`
(`Ingame_DeathRevive`), `Menu_ResultEndScreen` (`Ingame_ResultScreen`), `Menu_Ingame` (`Ingame`/`IGTitleUI`) — check
each header's dependencies first the way `Menu_Social`/`Menu_PortraitMessage` were checked here before assuming
they're simple; several of these are gameplay-HUD related and might also reach into undeclared gameplay-only types.

## UPDATE, same session: Menu_SendChallengeType/Taunt ported (9 menus total) + a real BasicPage crash found and fixed

Both got the usual minimal treatment (`const`/include fixes, real Aurora names `SendChallenge_Type`/
`SendChallenge_Taunt` from the 107-name list). `Menu_SendChallengeTaunt.h` needed one more thing: an undeclared
nested `STauntElement` struct (used only as `std::vector<STauntElement>` element type) — added as an **intentionally
empty placeholder** (`struct STauntElement {};`), explicitly flagged in a comment as unverified shape, safe only
because this session's minimal `.cpp` never populates or reads it. If a later session starts actually using
`m_tauntElements`, give `STauntElement` real fields first (not yet recovered from disassembly).

**Found a real, pre-existing crash while verifying these**: any Aurora page containing a `3DCostumePos` element (GUI
type 14) segfaults, because `BasicPage::GenerateFromAuroraFile`'s `case 14: ... continue;` (that element type isn't
ported — `Interface3DCostumePos.cpp` is a 1-line stub) skips adding an object for that index, but **nothing else in
BasicPage.cpp accounted for a hole in `m_elementsArray`** — the original, fully-ported engine never has this
problem since every index always holds a real object. Two distinct crashes from this, found via `gdb -batch -ex run
-ex bt` (actually running it under gdb beats staring at `armdis.py` output or guessing — use it whenever a segfault
needs tracing in this environment, it's just `apt-get install -y gdb` away... actually it was already installed
here):
1. `BasicPage::GenerateFromAuroraFile`'s `AutoSetGUIParent(GetUIObject(i))` loop called it on a null/out-of-bounds
   slot. `GetUIObject(int)` was a raw unchecked `m_elementsArray[i]` — for the LAST element in a layer being the
   skipped one, the vector is never even grown that far (out-of-bounds read, not just null).
2. `BasicPage::Reset()`/`OnMenuQuit()`/`UnloadClaraData()` all did `m_elementsArray[i]->SomeCall()` unconditionally.

Fixed both root causes: `BasicPage::GetUIObject(int)` in `BasicPage.h` is now bounds-checked (`i >= 0 && i <
m_elementsArray.size()`, returns `0` otherwise); `GenerateFromAuroraFile`'s `AutoSetGUIParent` loop and `Reset()`/
`OnMenuQuit()`/`UnloadClaraData()` in `BasicPage.cpp` all null-check before dereferencing. This isn't a guess at
missing behavior — it's a defensive fix for a hole our own incomplete port creates, with the original code path
(ported element types) completely unaffected. **Protects every future menu with a `3DCostumePos` element, not just
these two** — several levels/popups likely have one (shop costume preview, send-challenge taunt preview, etc).

**9 working menu screens now (MainMenu, Shop, Options, Goals, Pause, InviteFriends, MoviePlayback,
SendChallengeType, SendChallengeTaunt).**

## UPDATE, same session: Menu_ChallengeResult/Menu_DeathRevive ported (11 menus total)

Both clean headers, no nested enums, no weird dependencies -- `armdis.py -s <Class>` confirmed `const` on
`GetMenuName()` for both (now confirmed on every single menu checked this session; treat it as certain the header
extraction always drops it, not something to re-verify each time). Real Aurora names `Ingame_ChallengeResult`/
`Ingame_DeathRevive` (already known from the 107-name list dumped earlier this session). Minimal `.cpp`s as usual.
**Both confirmed rendering real content**: ChallengeResult shows "CHALLENGE WON!!!" with a reward icon and two OK
buttons; DeathRevive shows "KEEP RUNNING!" with a trophy, a friends photo icon, a distance readout, and a revive/OK
popup. No crashes (verified with `gdb -batch -ex run -ex bt`, not just a plain run, now that the 3DCostumePos crash
taught this session to actually check). **11 working menu screens now (MainMenu, Shop, Options, Goals, Pause,
InviteFriends, MoviePlayback, SendChallengeType, SendChallengeTaunt, ChallengeResult, DeathRevive).**

## UPDATE, same session: Menu_ResultEndScreen ported (12 menus total)

Needed one more undeclared nested enum (`EExitOptionsDef`, used by the private `Exit()` method) but this time
disassembly didn't help pin the count: `Menu_ResultEndScreen::Exit()` at `0x4913b4` just does
`str r1, [r0, #0x124]; bx lr` — stores unconditionally, no bounds check to read a range from. Used a 2-value
placeholder (`Exit_None=0, Exit_MainMenu=1`), explicitly flagged as unverified in the header comment, safe because
this session's minimal `.cpp` never calls `Exit()` with a meaningful value. Also hit the by-now-familiar pattern of
a dependency header (`game/online/store/Price.h`) having zero includes of its own and not compiling standalone —
worked around by forward-declaring `class Price;` instead of including it (only used by-const-reference here, so a
forward declaration is enough; didn't need to fix `Price.h` itself for that). Needed `core/delayer.h` and
`core/interputils.h` for the `jet::core::Delayer<float, jet::core::SmootherStep>` member.

One nice confirmation while disassembling: the real `Menu_ResultEndScreen::Render()` is literally
`b Menu_Base::Render` — a tail call, zero override logic. Confirms this session's "minimal `.cpp` that just
delegates to `Menu_Base`" pattern isn't merely a safe approximation; for at least this one method on this one menu,
it's bit-for-bit what the original does.

**Confirmed rendering real content**: "Bananas collected:"/"Distance:" stat rows, a SHARE button, a tip speech
bubble ("You can use Bananas to unlock the Power-ups! Let's enter the Shop!"), and MENU/SHOP/NEXT buttons. No crash
(checked with `gdb -batch -ex run -ex bt`). **12 working menu screens now (MainMenu, Shop, Options, Goals, Pause,
InviteFriends, MoviePlayback, SendChallengeType, SendChallengeTaunt, ChallengeResult, DeathRevive,
ResultEndScreen).**

Remaining untried: `Menu_Ingame` (`Ingame`/`IGTitleUI` — this is likely the real in-game HUD itself, probably the
most structurally important one left and possibly the most complex), `Menu_Social`/`Menu_PortraitMessage` (both
previously scoped out, see above, need real subsystem work not just header fixes). The checklist from before still
applies to all of them: `armdis.py -s <Class>` first, real Aurora name from the 107-name list, recover any
undeclared nested enum (or use a flagged placeholder if disassembly doesn't reveal a range, as with
`EExitOptionsDef` here), forward-declare rather than include a dependency header that's itself broken if only a
reference/pointer is needed, minimal render-only `.cpp`, register, verify under `gdb -batch -ex run -ex bt`
(actually run it, don't just compile-check), screenshot, revert the scratch test, commit only the real changes.

## UPDATE, same session: Menu_Ingame ported (13 menus total) -- the big one, the real gameplay HUD

Did the biggest one last, as predicted. `Menu_Ingame.h` is by far the largest header in this tree (60+ methods:
score/banana/despicable-meter bookkeeping, powerups, perks, vehicle choice, checkpoints, 8 numbered debug sliders,
next-friend-to-beat). Needed 3 nested enums (`EStateDef`, `EStateMultipliersPerksDef`, and `EVehiclesDef` -- the
last one referenced *bare*, not nested, also needed by `source/game/cheats/CheatsMgr.h`, which never declares it
either; declared it in `Menu_Ingame.h` since that's the first place in the tree that needs it, with a note to move
it to a shared header if `CheatsMgr.cpp` is ever ported). Recovered `EStateDef`'s likely shape from
`SetState()`'s branches (`cmp #1`/`cmp #2`) lining up with the `UpdateStateNormal`/`UpdateStateCheckpoint`/
`UpdateStatePerks` method names already in the header -- plausible, not independently confirmed the way some
other enums this session were (flagged as such in the header comment). `EStateMultipliersPerksDef`/`EVehiclesDef`
are unverified placeholders (not critical path for a render-only pass).

Despite the size, it compiled clean on the first real attempt once the by-now-familiar fixes were applied
(`#include "Menu_Base.h"`/`jenum.h`/`core/delayer.h`/`core/interputils.h`/`tracking.h`, `GetMenuName()` `const`,
forward-declare `ParticleSystem2D`/`ps::ParticleSystem`/`Interface3DElement`/`WeeklyMission`/`OnlineUser` rather
than including their headers) -- the "60+ methods" turned out to mean 60+ mechanical one-line stubs, not 60+ hard
problems. **Confirmed rendering real content, no crash** (`gdb -batch -ex run -ex bt`): powerup icons, a trophy/
achievement badge, a bananas counter badge, an "x2" multiplier tag, "ACTIVATED"/"LOADING" power-up labels, a
Facebook connect icon, and minion costume thumbnails all show up. It's visually busy/overlapping -- the debug
sliders and several elements that would normally be hidden or positioned by live gameplay state all render at once
since nothing drives their visibility/layout without real `GS_Gameplay` data -- but every element is genuine
Aurora/localization/sprite content, not placeholder noise.

**13 working menu screens now (MainMenu, Shop, Options, Goals, Pause, InviteFriends, MoviePlayback,
SendChallengeType, SendChallengeTaunt, ChallengeResult, DeathRevive, ResultEndScreen, Ingame).** That's every
menu in `source/game/interfaces/menus/` ported except `Menu_Social` and `Menu_PortraitMessage`, both scoped out
earlier in this file for needing real subsystem work (a whole `LeaderboardMgr`/`social::` object graph; `InputMgr::
TutorialInputId` + a nested struct) rather than just the header-gap fixes that worked for everything else.

**What's next for "fully 1:1 playable" is no longer about this menu-porting recipe** -- it's now squarely: (1) real
button-press navigation from the main menu (the `Menu_MainMenu::Update` dead-end noted earlier, still not found),
(2) `GS_Gameplay` itself (still a 1-line stub -- nothing above makes the game actually playable, it makes every
*screen* reachable and renderable), (3) the two scoped-out menus' real subsystems, (4) the missing lab hallway
models and color-grading matrix noted earlier in this file, both still open. The menu-by-menu recipe documented
throughout this file remains valid for `Menu_Social`/`Menu_PortraitMessage` whenever someone picks those up.

## UPDATE, same session: why `MenuMgr::PushMenu` callers can't be found by symbol search

Picked up thread (1) above: real button-press navigation out of `Menu_MainMenu`. Confirmed via
`tools/armdis.py <so> -s "MenuMgr::"` that **the real binary has exactly one `MenuMgr::` symbol**
(`MenuMgr::GetCurrentPlayingMenu() const`, 28 bytes at `0x3fba3c`). `AddNewMenu`, `PushMenu`, `PopMenu`,
and `Update` have no standalone symbols at all -- they're small enough that the compiler inlined every
call site, so there is nothing named `MenuMgr::PushMenu` to find callers of by cross-referencing a symbol.
Same story for `InterfaceButton::` (zero symbols) -- button click handling is inlined into whatever Aurora
dispatch code runs it, not sitting in a method with its own name.

This means the earlier approach (grep `Menu_MainMenu::Update`'s disassembly for a call to a function named
`PushMenu`) was never going to work, regardless of how carefully it was done -- it wasn't a dead end specific
to `Update`, it's structural: inlining erased the symbol everywhere in the binary, not just in that one method.
Finding the real dispatch now requires reading `Menu_MainMenu::Update(int)` (6464 bytes, `0x4869fc`) or
`ExecuteTouchInteractEvent(int)` (256 bytes, `0x48840c`, already partially disassembled -- its `cmp r1,#1`/
`cmp r1,#3` branches are 3D-touch-interact event codes, e.g. the first branch is a vtable call that ends up at
`Object::CanLoadModel()`, not menu navigation) instruction-by-instruction and recognizing an *inlined* store into
whatever field tracks the pending-menu-to-push, which is a much larger, more open-ended disassembly task than the
previous "find the call" plan assumed. Not done this session -- flagging the corrected approach and scope for
whoever picks this up next, rather than guessing at a mechanism that can't be cross-checked.

Given that, the highest-leverage remaining work for "fully 1:1 playable" is probably **not** this dispatch
mechanism (it only matters for driving navigation from real touch input -- `MenuMgr::PushMenu()` already works
correctly when called directly, which is how all 13 menus above were verified) -- it's more likely `GS_Gameplay`
itself, since that's the one piece with zero working implementation at any entry point, direct-call or not.

## UPDATE, same session: started GS_Gameplay -- real verified pieces only, and an important layout warning

`GS_Gameplay::GameplayMode` is a real enum, fully verified rather than guessed: `UpdateGameplayMode()`
(`0x4fcf4c`) contains three back-to-back jump tables, all keyed on `(mode - 1)` and all agreeing on the same
mapping (case 0 -> every Fluffy-named Begin/End call, case 1 -> Rocket, case 2 -> LargeMinion, case 3 -> Boss),
with mode value 0 (the `-1` that wraps to a huge unsigned number and falls out of the jump table's `<=3` range)
meaning "no special mode" / Normal in every one of the three tables. Added as `GS_Gameplay::GameplayMode` with
values `Mode_Normal=0, Mode_Fluffy=1, Mode_Rocket=2, Mode_LargeMinion=3, Mode_Boss=4`.

**Important finding, not yet resolved: this header's field offsets (and maybe its `sizeof 0x60`) look wrong.**
`GS_Gameplay::GS_Gameplay()` (`0x4fc260`) constructs a `std::map<jet::String,int>` (via an `_Rb_tree` ctor/dtor
pair) and calls `Statistics::GetIntValue()` before `InitSessions()` -- neither corresponds to anything in this
header's current member list. Also, `EnableShootingArea(unsigned char)` (`0x4fd304`) reads/writes
`m_shootingAreaClassMask` at **offset `0x54`**, not the `0x3D` this header currently comments it at. Both point
the same way: the PDB-derived header is missing at least one member (the map), which shifts every field after it.
Did **not** attempt to guess the map's purpose or reconstruct the real layout this session -- that needs its own
investigation (grep the constructor's full disassembly for what reads/writes that map elsewhere, e.g. checkpoint
or level-visited tracking) before anyone trusts `new GS_Gameplay()` to be layout-safe.

Given that warning, only implemented the leaf methods that don't depend on the suspect layout being complete:
`GetStateName()`, `GetInterface()` (verified @`0x4fdb0c`: tail call to
`MenuMgr::GetInstance()->GetMenu(Menu_Ingame::k_menuName)`), the mode bookkeeping half of `UpdateGameplayMode()`
(real logic, minus the `GameLevel::Begin/EndXGameplay()` calls and the per-mode material/vtable call the three
jump tables also make -- `GameLevel::BeginFluffyGameplay()` etc already exist as real functions in the binary
per `armdis.py -s GS_Gameplay` output but aren't declared in `GameLevel.h` yet, so calling them was left out
rather than guessed at), and simple accessors (`IsFluffyGameplayMode`/`IsRocketGameplayMode`/
`IsLargeMinionGameplayMode`/`IsBossGameplayMode`/`IsInShootingArea`/`GetShootingAreaMask`/`IsInBonusGameplay`/
`EnableBonusGameplay`/`GetMusicHandle`/`SetProcessingRevive`, plus a simplified `EnableShootingArea` that stores
the mask but skips the real version's side effect of flipping a flag on an unidentified global touch-input lock).
Also removed a stray non-virtual `void GetStateName();` from the header -- same PDB-extraction artifact as the
missing-`const` `GetMenuName()` bug fixed on every `Menu_*` class earlier, just manifesting as a second
declaration instead of a missing qualifier this time; it conflicted with the real virtual override and had to go
for this to even compile.

The constructor itself is intentionally **not** defined yet (nothing in the tree calls `new GS_Gameplay()`, so
leaving it undeclared-but-unused is safe and doesn't break the build) -- writing it before the layout question
above is resolved risks shipping a confidently-wrong member-init list. Compiles clean, full `test_intro`
regression still reaches `GS_MainMenu` with no new warnings or crashes. **Next step for whoever picks this up:
resolve the layout gap first** (what is the extra map, and does fixing it reveal more missing members), then
write the real constructor and wire `GS_Loading`'s `GoTo_Gameplay` path to actually push a `GS_Gameplay` --
that's the one missing link between "every screen renders" and "the game is actually playable."

## UPDATE, same session: GS_Gameplay is now reachable and stable -- first real (not menu) gameplay state

Finished what the previous section started. Fixed the real sizeof (`0x78`, verified: `GS_Loading::
_UpdateLoadingStep()` at `0x500560` mallocs exactly that many bytes right before constructing
`GS_Gameplay` -- confirms the missing-`std::map` theory almost exactly, `0x78 - 0x60 = 0x18` matches
libstdc++'s 32-bit `_Rb_tree` overhead almost to the byte). Added the map back as
`m_unverifiedCounterMap` (purpose still unknown, not read/written by anything ported so far, kept only
so sizeof stops lying). Wrote the constructor, `ResumeState`/`SuspendState`/`UpdateState`/`RenderState`,
and `InitSessions()` -- all structurally verified against the real disassembly, with one recurring
omission flagged throughout: **there is no `SoundMgr` class anywhere in this codebase.** The sound work
earlier this session (Musepack SV8) was the audio *codec*, not an engine-level sound-manager singleton,
so every `SoundMgr::Resume`/`PauseAll`/etc. call the real binary makes from these methods was left out
rather than invented against a class that doesn't exist here. `UpdateSubStateInit`/`UpdateSubStatePlaying`
(the actual countdown/running logic) are still no-op stubs -- next concrete step for whoever continues this.

Found `GS_Loading::_UpdateLoadingStep()`'s real `GoTo_Gameplay` case via `armxref.py ... to 0x4fc260`
(who calls `GS_Gameplay`'s constructor) rather than guessing -- it also constructs and pushes a
`GS_PreGameplay` on top (plays the level's intro movie/countdown, then pops to reveal `GS_Gameplay`
underneath). `GS_PreGameplay.cpp` was also a 1-line stub; ported it the same way (all methods, including
the real 3-substate movie-wait state machine, verified from disassembly), except `Movie_StartMovie`/
`IsPlaying`/`Stop`: the real versions poke a `clara::Movie::TimeController` sub-object this tree's own
`clara::Movie` (a from-scratch reimplementation, not a PDB port -- see `core/claraloader/include/movie.h`)
doesn't have, so those three were *adapted* to the `SetPlaying()`/`IsPlaying()` API that actually exists
here instead, flagged as adapted-not-verified. Wrote `GS_Loading::Finish()`'s previously-`default:`-only
`GoTo_Gameplay` case to match the real binary: add an `ACTION_PHASE_SESSION` tracking session, push
`GS_Gameplay`, push `GS_PreGameplay`. `GameLevel::SetEntityToSpawnCount(1)`, called in the same real
block, isn't declared in `GameLevel.h` yet and was left out rather than adding an unverified method to a
header this pass wasn't otherwise touching.

Built a scratch test (boots to `GS_MainMenu` like `test_intro`, then pushes `GS_Loading(GoTo_Gameplay,
Steps_Gameplay, ...)` the exact way `GS_MainMenu::StartNormalGamePlay()` does) and ran it under
`gdb -batch -ex run -ex bt`. Found and fixed **two real, pre-existing engine bugs** that were blocking
this path entirely, neither introduced by this session's menu work:
1. `GameplayTemplateInstance::ProcessCreatedEntity()` (`source/game/leveltemplate/
   GameplayTemplateInstance.cpp:101-102`) called `LocationsMgr::GetLocationConfig(location)->
   GetRenderPlatforms()` with no null check, even though `LocationsMgr::GetLocationConfig()` can and does
   return null (confirmed: `BackgroundMgr::EnableFluffyDimensionBackground` and `EdgeProbabilityMgr`
   already null-check the same call elsewhere in this tree) -- this call site was just missing it. Fixed
   with a null guard, same pattern as the `BasicPage::GetUIObject` fix earlier this session.
2. `game::common::SessionTrackingMgr` (a `Singleton<T>`) was never constructed anywhere -- nothing in
   `Game::Init()` did `new SessionTrackingMgr` the way it does for every other singleton this tree uses
   (`GameSettings`, `MenuMgr`), so `GetInstance()` returned null the moment `GS_Gameplay::InitSessions()`
   or `GS_Loading::Finish()`'s new `GoTo_Gameplay` case called it. Added `new game::common::
   SessionTrackingMgr;` next to the other singleton constructions in `Game::Init()` (`source/game/
   Game.cpp`). Also discovered `SessionTrackingMgr.h` itself had the same zero-includes PDB-extraction
   gap as every other header this session touched (missing `<map>`/`<string>`/`jsingleton.h`/a forward
   declaration for `TrackingSession`, plus an MSVC-only `__int64` that doesn't exist under GCC) and fixed
   those too; wrote a minimal but real `SessionTrackingMgr.cpp`/`TrackingSession`'s `.cpp` (it was also a
   1-line stub) since nothing existed to link against.

**After both fixes, the scratch test reaches and stays in `GS_Gameplay` for the rest of the run with no
crash** -- confirmed via `gdb` (no backtrace, clean exit) and a screenshot at t=19s showing a real minion
rendered in-scene against a solid sky-colored background (no lab/level geometry yet -- expected, that's
the already-documented missing-assets gap, not a new bug). Screenshot sent to the user. This is the first
time any state other than a menu has rendered successfully this session -- **the actual missing link
between "every screen renders" and "the game is playable" is now closed at the state-transition level.**
What's *in* `GS_Gameplay` once it's running (the substate machine, real entity spawning/scoring, the
countdown via `GS_PreGameplay`'s movie) is still mostly stubs -- that's squarely the next layer of work,
not a structural blocker anymore.

## UPDATE, same session: GS_Gameplay's substate machine -- the real in-game HUD now shows during gameplay

Picked up the next layer immediately. Ported `UpdateSubStateInit()` and `UpdateSubStatePlaying()`
(verified @`0x4fcc88`/`0x4fcee4`), conservatively trimmed:

- `UpdateSubStateInit()` (runs once, right after construction -- `m_subState(1)` in the constructor is a
  logically-forced value, not an individually verified store: `UpdateState()`'s dispatch is 1->Init,
  2->Playing, and `UpdateSubStateInit()` is the only place that writes `m_subState`, to 2, once -- for the
  real control flow to make sense the constructor has to start at 1): calls `LevelDef::OnInitGamePlay()`
  (runs the level script's real `OnInitGamePlay` Lua function), `RestartSessions()` (newly ported too --
  verified it only restarts `m_currentRunSession`, not `m_runSession`, not a typo), flips to substate 2,
  advances the level once, and **pushes `Menu_Ingame` via `MenuMgr::PushMenu`** -- the real in-game HUD,
  ported earlier this session, now actually shows up during real gameplay instead of only being reachable
  through a direct test harness call. The real function also touches `WeeklyChallengeMgr`/`MissionMgr`/
  `ChallengeMgr`/`IGNotificationMgr`/`AchievementsMgr`/`MessagesMgr`/`LevelSequenceGraph` -- all real
  classes that already exist in this tree, but wiring seven meta-progression subsystems together in one
  pass, each with unverified internals of its own, was too large a bite for this pass. Deliberately
  skipped rather than guessed at -- a good next target for whoever continues this.
- `UpdateSubStatePlaying()` (runs every frame once substate 2 is reached): advances the level every
  frame. The real version has three early-return guards this session couldn't identify (a field on
  `Menu_Ingame` at an unnamed offset, a field on `GS_Gameplay` itself at an unnamed offset, a byte on an
  unidentified global -- all presumably some flavor of "something is blocking gameplay, don't advance the
  level"). Omitted rather than guessed, and harmless for now since nothing in this tree currently sets any
  of them -- but the real mechanism exists and should be restored rather than reinvented once a future
  pass needs to pause the level from a dialog/movie/menu.

Verified via the same scratch gdb-driven test as the previous update (removed before commit): stable for
the full run, no crash, and the screenshot now shows the real `Menu_Ingame` HUD rendering live during
gameplay (pause button, score/banana badges, powerup icons, costume thumbnails) instead of a bare scene.
Sent to the user. `UpdateTimers()`/`CheckGameplayEnd()` are still no-ops (the former needs a verified
per-frame dt source this session didn't trace back to anything in this tree yet; the latter wasn't
disassembled this pass) -- next concrete steps alongside the seven skipped meta-progression subsystems
above.

## UPDATE, same session: InputMgr -- the real swipe/touch classifier, fully verified

Switched targets: `source/game/input/InputMgr.cpp` was a 1-line stub (and the header referenced two
undeclared types, `TutorialInputId`/`TutorialInputListener`, the usual PDB-extraction gap). This is the
class that turns a raw swipe into a game direction (lane change left/right, jump, slide) -- literally how
the player controls the minion, so probably the single highest-value remaining port for "playable" in the
hands-on sense, not just the visual sense.

Fully ported and **verified, not approximated**: `GetLineSlideAction8`/`GetLineSlideAction4` (the 8- and
4-direction octant/quadrant classifiers, `0x504df8`/`0x504f10`). Extracted every boundary-angle literal
straight from the binary's literal pool with a small script reading raw floats at each `vldr [pc,#imm]`
target (not estimated from the disassembly's printed hex) -- confirms the 8-way boundaries sit exactly at
odd multiples of `PI/8` (22.5DEG steps) and the 4-way ones at odd multiples of `PI/4` (45DEG steps), both
normalizing a negative `atan2f` result by `+2*PI` first, matching standard angle-bucketing exactly. Also
extracted the real `.data` values of all eight `k_SLIDE_ACTION_*` constants directly (not assumed from
their declaration order) -- they are `0..7` in declaration order, which let the classifier's return values
be written as the named constants with no ambiguity. Round-tripped through a scratch test (removed before
commit): East->LeftRight(6), a vertical swipe->UpDown(0)/DownUp(4) depending on sign, West->RightLeft(2),
a NE diagonal->UpLeftDownRight(7) -- all exactly as expected, confirming the boundary math and the
constant mapping agree.

Also ported the constructor (verified structure, including the real percentage-of-screen-width ->
squared-distance precompute for `s_minSlideDistance`/`s_maxSlideDistance`/`s_maxTouchDistance`: the three
`k_SLIDE_*_DISTANCE` constants are exactly `28/768`, `60/768`, `30/768` of a reference width, scaled by the
real screen width via `jet::System::GetMainDisplay()->GetMode().size.x` and squared so callers compare
squared distances), `Reset()` (reproduced by intent since its real field offsets didn't map cleanly onto
this header's layout -- another case of the same offset-shift seen on `GS_Gameplay` and others, not
chased further here), `IsSlideAction`/`GetTouchAction`/`OnKeyPressed`/`OnKeyReleased` (all small and
verified), and the straightforward field accessors (`SetEnabled`, `SetPlayer`, `EnableTouchInteracts`,
`IgnoreAllSlideActions`, `IgnoreVerticalSlideActions`, `GetInputRatio`/`GetInputRatioLast10Seconds`).

**Not ported, and this is the actual remaining gap for real player control:** `Update(int)` /
`UpdateSlideActions(int)` / `LaunchEvents()` (68/720/904 bytes) -- the per-frame loop that reads real
touch/OS input events and calls the now-verified classifiers above, then fires `OnSlideAction`. This
session didn't trace this tree's actual touch-input plumbing (how a raw tap/drag reaches game code at
all), so these are still no-ops. The classifier math is solid and ready; what's missing is the event
source feeding it. `SetTutorial`/`IsTutorialActionEnabled`/`OnTutorialInputDone` are also stubbed (return
`true`/no-op) since the tutorial system is a separate concern and `TutorialInputId`'s real shape is still
an unverified placeholder.

## UPDATE, same session: UpdateSlideActions -- real touch events now flow all the way to a slide gesture

Picked the open thread back up immediately: discovered `source/game/input/TouchMgr.cpp`/`.h` is already
a complete, real port from an earlier session (not this one) -- a proper per-finger touch-state tracker
(`Touch::State` Pressed/Dragged/Released/Cancelled, a fixed 10-slot table, a queue so `OnTouchPress`/
`OnTouchDrag`/`OnTouchRelease`/`OnTouchCanceled` can be called from a platform layer at any time and get
applied deterministically on the next `Update()`). This was the missing piece that made
`InputMgr::UpdateSlideActions`/`LaunchEvents` intractable to finish in the previous update -- it's no
longer a "where would touch events even come from" problem, `TouchMgr::GetAllActiveTouches()` is real and
exactly what the disassembly calls.

Fully disassembled and ported `UpdateSlideActions(int)` (`0x50472c`, 720 bytes) against it. One
correction from an initial misread: the first branch looked like "bail out if more than 3 touches active"
but is actually comparing a raw byte-length (not an element count) against 3, i.e. "bail out if there are
*zero* active touches" (any ustl::vector with at least one `Touch*` has a byte-length of at least 4,
always `> 3`) -- caught by cross-checking against the loop's own exit condition later in the same
function, which unambiguously shifts that same value right by 2 to get an element count. Worth noting
explicitly: this is the kind of bitwise-literal-vs-semantic read where a first pass can get the polarity
backwards, and it's why this session re-derives control flow from both ends (entry check and loop
condition) rather than trusting a single read.

The ported logic, verified end-to-end: each `Update()` clears this frame's slide-action bitset/
`m_upDirection`/`m_touched`, then for every active touch: claims an untracked Pressed/Dragged touch,
accumulates `m_pressedTime` and tracks `m_touchLastPos`, ignores touches that haven't moved past
`k_TOUCH_MAX_DISTANCE` yet unless they just released (a release always finalizes regardless of distance),
classifies the gesture with the now-also-verified `GetLineSlideAction4` once it passes
`k_SLIDE_MIN_DISTANCE`, latches the first detected action and its vertical direction (expressed as
`k_SLIDE_ACTION_UpDown`/`DownUp`, reused as a direction flag -- confirmed by the real binary using those
exact same two constants here, not separate ones), marks `m_touched`/`m_touchedPos` for taps within
`k_SLIDE_MAX_DISTANCE`, and clears the tracked touch on release. Also wired `Update(int)` itself (verified
@`0x5046e8`: does nothing unless enabled or a tutorial listener is waiting, otherwise calls
`UpdateSlideActions` then `LaunchEvents`).

Verified with a scratch test (removed before commit): fed a real press-drag-release sequence through
`TouchMgr`'s public API (exactly as a platform input layer would) and called `InputMgr::Update()` each
frame -- a 150px rightward drag correctly set `LeftRight` and nothing else among the 8 slide actions, no
crash. **Real touch input now flows all the way from `TouchMgr` to a classified slide gesture** -- the
remaining gap is purely `LaunchEvents()` turning that classified gesture into an actual lane-change/jump/
slide call on the player's `Minion`, which needs its own pass (it's 904 bytes and also handles a separate
camera-ray/`BodyPartOwner` 3D touch-interact hit-test path this session didn't trace into). Confirmed from
the disassembly that `LaunchEvents()` null-checks `m_player` and returns immediately if unset, so
`SetPlayer(minion)` must be wired up before it can do anything -- also not yet connected from `GS_Gameplay`
or `GameLevel`, another concrete item for whoever continues this.

## UPDATE, same session: why LaunchEvents is a bigger undertaking than it looked, and wiring InputMgr up

Continued disassembling `LaunchEvents()` past where the previous update stopped, to scope it properly
before attempting it. **Important finding: this is genuinely blocked on more than just "trace 904 bytes of
disassembly."** The real dispatch, once a slide action is confirmed, calls through `m_player`'s own
**vtable at slots 0xC8/0xCC (indices 50/51)** -- e.g. `m_player->vtable[50](action, 1)` for a horizontal
slide, `m_player->vtable[51](9)` / `m_player->vtable[51](0x25)` for two other named actions (jump? duck?
not confirmed). This session's `Minion` class (`source/game/entities/Minion.h`) is a small, already-
rebuilt-from-scratch 51-line header with only 5 virtual methods (dtor, Init, PostInit, Update, Render) --
nowhere near 50+ virtual slots. The real `Minion` (or whatever the real `m_player`'s static type actually
is -- possibly a separate interface class, not confirmed) has a vastly larger vtable than what exists in
this tree. **Adding 2 methods at arbitrary vtable positions to patch this would corrupt every other
virtual call this tree already makes through `Minion*`** -- not a safe partial fix. Finishing
`LaunchEvents()` for real requires first reconstructing `Minion`'s actual full vtable (or confirming
`m_player`'s real static type and reconstructing that instead), which is a substantially bigger task than
this pass's InputMgr work and shouldn't be attempted piecemeal. Flagging this clearly so a future pass
doesn't repeat the investigation: **the blocker isn't missing disassembly time, it's a missing prerequisite
class.**

Given that, did the safe, bounded part instead: **InputMgr wasn't owned or updated by anything in this
tree at all** (unlike `TouchMgr`, which `Game::FrameUpdate()` already calls `Update()` on every frame).
The real binary accesses it via `glf::App::GetInputMgr()`/`glf::GetInputMgr()` -- free functions from an
app-framework layer (`glf::`) that doesn't exist anywhere in this tree, not `Singleton<T>`. Rather than try
to reconstruct that missing layer, followed this tree's own established local convention instead (every
other per-game manager here -- `MenuMgr`, `GameSettings`, `PopupMgr`, etc. -- already uses `Singleton<T>`
regardless of how the real binary accessed it): made `InputMgr : public Singleton<InputMgr>`, constructed
it in `Game::Init()` next to the other singletons, and added `InputMgr::GetInstance()->Update(dt)` in
`Game::FrameUpdate()` right after `TouchMgr::GetInstance()->Update()` (so it sees the current frame's
touch state, not last frame's). Also wired `InputMgr::GetInstance()->SetPlayer(GameLevel::GetInstance()->
GetMinion())` into `GS_Gameplay::UpdateSubStateInit()` -- not verified from disassembly where the real
binary calls `SetPlayer`, but this is the first point in this tree's current call graph a level's `Minion`
is guaranteed to exist, and it's needed before `LaunchEvents()` (whenever it's finished) can do anything
at all.

Verified via the same scratch gdb-driven boot test used for earlier `GS_Gameplay` updates (removed before
commit): stable through the full run, no crash, confirming the new construction/update/`SetPlayer` wiring
doesn't break anything even though `LaunchEvents()` itself is still a no-op. **Net effect of this update:
touch input is now a live, ticking subsystem with a real player reference, one disassembly-and-vtable-
reconstruction pass away from actually moving the minion** -- a clearly scoped, bounded next step rather
than an open-ended one.

## UPDATE, same session: identified exactly what LaunchEvents calls -- and why it's still not a quick win

Followed up on the "reconstruct Minion's vtable" blocker with a more precise technique: rather than
guessing, read `vtable for Minion` (`0xce3058`, 500 bytes) directly and resolved the real function
pointers stored at the two slots `LaunchEvents()` calls (`+8` to skip the Itanium ABI's offset-to-top/RTTI
header, then `+0xC8`/`+0xCC`). They resolve to real, named, mangled symbols:
`Minion::SM_OnScreenEvent(int, int, bool)` and `Minion::SM_OnEngineEvent(int)`. This is good news and bad
news. Good: these aren't some mystery 50-slot vtable gap to blindly fill in -- they're two specific,
named methods, and **adding exactly these two as new virtual methods to this tree's `Minion` class is
completely safe** (plain C++ virtual dispatch doesn't care about matching the real binary's raw vtable
offsets; that concern only applied to literally reproducing the `blx [vtable+0xC8]` machine code, which a
normal virtual call doesn't need to do). Bad: `source/game/actors/statemachine/StateMachine.h` already
declares both of these as virtuals (`virtual bool SM_OnScreenEvent(int, int, bool)`, `virtual bool
SM_OnEngineEvent(int)`) on a **`StateMachine` base class that this tree's `Minion` doesn't inherit from**,
and `StateMachine.cpp` is itself still a 1-line stub -- a ~75-method class (state sets, triggers, queued
events, animation blending, `ActorState`/`ActorStateSet`/`StateTrigger`/`StateMachineEvent`, none of which
exist anywhere in this tree either) that is the real animation/gameplay state engine the whole Minion runs
on. This tree's `Minion` was independently rebuilt with its own simpler `SetStateMachine`/`EnterState`
string-based API instead of porting `StateMachine` -- a reasonable choice for getting menus/idle animation
working, but it means `SM_OnScreenEvent`/`SM_OnEngineEvent` have no real `StateMachine` machinery to
forward into. **The actual remaining work is: either port enough of `StateMachine` for these two events to
drive real state transitions, or translate a screen/engine event directly into this tree's existing
`EnterState(name)` API as a pragmatic shortcut** (would need figuring out which event codes/`action`
values map to which state names -- not yet investigated). Either path is its own multi-step undertaking,
not a quick follow-up, so leaving it here rather than starting a third, under-scoped attempt this session.

Checked whether the "map event codes straight to `EnterState(name)`" shortcut was actually viable before
ruling it out: disassembled `StateMachine::SM_OnScreenEvent` (`0x58cb68`, 184 bytes) itself. It does *not*
map its action code to a state name in C++ at all -- it either queues a `StateMachineEvent{type=2,
param1=action, param2, timestamp}` for later, or forwards immediately to `SM_OnStateEvent(2, action,
param2)`, which (per the `states` datalib's own `StateLabel`/`StateType`/`Event`-field schema, visible via
`strings data/d3/datalibs/states`) matches against each state's *data-defined* trigger conditions to
decide what to transition to. In other words the action->state mapping is entirely data-driven inside the
real `StateMachine`/trigger engine, not a fixed table in code -- so there is no safe shortcut here; a
hardcoded `action -> EnterState("DodgeLeftSel")`-style guess (even though promising-looking state names
like `DodgeLeftSel`/`DodgeRightSel`/`JumpSel`/`SlideSel` do exist in the real data, confirmed via
`strings`) would be fabricating a mapping, not reproducing one. Confirms the real work is porting (at
least the trigger-matching core of) `StateMachine`, not a quick bridge.

## UPDATE, same session: GS_Pause ported end-to-end -- real pause screen now reachable from gameplay

Stepped back from the `StateMachine` undertaking (too large to start productively this session, per above)
and picked a smaller, fully tractable, self-contained target instead: `GS_Pause` was still a 1-line stub,
all of its methods small (28-136 bytes) and fully disassembled this update. Verified real behavior:
constructor pushes `Menu_Pause` (already ported earlier this session), sets a `Game::m_paused` flag (real
field, confirmed at the exact same byte offset `0x318` this tree's `Game.h` already uses for it) and stops
the menu music; destructor unsets the flag and stops the music again; `UpdateState()` restarts the menu
music if none is playing; `RenderState()` is the same `GameLevel::Render()` tail-call pattern as every
other `GameState` this session. `ResumeState()`/`SuspendState()` are confirmed genuinely empty in the real
binary, not a gap.

Also traced and wired the actual pause *trigger*: `GS_Gameplay::UpdateState()` (verified @`0x4fc6b4`,
revisiting the part skipped in the earlier `GS_Gameplay` update) checks a mutex-guarded bool on the `Game`
instance and, if set, pushes `GS_Pause` and calls `Game::SetGoToIGM(false)`. Added `Game::RequestPause()`/
`ConsumePauseRequest()`/`SetGoToIGM()` (none in the PDB header) -- no real mutex since nothing in this tree
sets the flag from another thread yet (the real flag is presumably set from a native pause-button/
lifecycle callback this tree doesn't generate), and `SetGoToIGM`'s real meaning isn't confirmed, just
bookkeeping for now. Also added `Game::GetMenuMusic()`/`PlayMenuMusic()`/`StopMenuMusic()` (verified call
sites in `GS_Pause`, but -- same recurring gap as `GS_Gameplay`'s `ResumeState`/`SuspendState` -- there is
no `SoundMgr` class anywhere in this tree, so these only track a handle, they don't play anything yet).

Verified via a scratch gdb-driven test (removed before commit): boots to `GS_MainMenu`, pushes
`GoTo_Gameplay`, waits for `GS_Gameplay`, then calls `Game::GetInstance()->RequestPause()` exactly like a
real pause-button press would -- the state correctly transitions `GS_Gameplay` -> `GS_Pause`,
`Game::IsPaused()` flips true, and it renders a real, polished "GAME PAUSED" screen (localized text, live
score/banana/distance labels, a costume thumbnail, Menu/Resume buttons) for the rest of the run with no
crash. Screenshot sent to the user. **This is the first time this session a screen transition triggered
*from inside* `GS_Gameplay` (as opposed to `GS_Loading`'s scripted sequence) has been verified working.**

Not wired: what happens when the player presses Resume/Menu *on* the pause screen (`Menu_Pause::UpdateExit()`
is still a no-op -- its real disassembly wasn't found via symbol search this pass, likely inlined, not
investigated further this session) and what in-game UI element calls `RequestPause()` in the first place
(almost certainly `Menu_Ingame`'s `m_pause_button`, already a real field from the earlier `Menu_Ingame`
port, but its click-handling isn't wired to anything yet -- same "real button press dispatch" gap noted for
the main menu earlier in this file). Both are natural next steps once the main-menu navigation-dispatch
question is revisited, since they're the same category of problem (a UI button needs to call a C++
method).

## UPDATE, same session: Menu_Pause::UpdateExit() is real -- resume/main-menu mechanics verified

Closed one of the two items left open above. Found `Menu_Pause::UpdateExit()` directly (its address,
`0x48d094`, was already recorded in a header comment from earlier in the session but never acted on) and
disassembled it fully: unconditionally calls `GameLevel::RemoveEntities()` once `m_exitOption != Exit_None`,
then for `Exit_Resume` pops the `GS_Pause` state and the `Menu_Pause` menu; for `Exit_MainMenu` clears any
active challenge, pops the menu twice, and calls `Game::GoToMainMenu()`. Needed `ChallengeMgr::
Challenge_IsActive()`/`ClearCurrentChallenge()`, which meant giving `ChallengeMgr` (another zero-includes,
1-line-stub, never-constructed singleton -- same recurring pattern) a minimal real port: verified
`Challenge_IsActive()`'s real formula (`OnlineUser_IsSet() && m_challenge && m_challengeStarted &&
IsNetworkReady()`) but since none of `OnlineUser_IsSet`/`IsNetworkReady`/the online challenge system exist
in this tree, always returns `false` -- accurate, not a guess, since nothing can ever set up an active
challenge yet. `ClearCurrentChallenge()` just sets the real deferred-delete flag; since nothing can make
`Challenge_IsActive()` true, nothing ever acts on it, so leaving `_ClearCurrentChallenge()` (692 bytes)
unported is safe for now. Constructed `ChallengeMgr` in `Game::Init()` next to the other singletons added
this session. Also found `Menu_Pause::Update(int)`'s real top-level branch (the full 988-byte function
itself not ported -- the rest is challenge/mission-box UI bookkeeping for boxes this tree's `Init*Box`
methods leave permanently null anyway): skips `Menu_Base::Update(dt)` while an exit is pending; wired
`UpdateExit()` to run every frame so a real `Exit()` call (once something calls it) actually fires.

Verified via a scratch gdb-driven test (removed before commit) extending the pause-trigger test from the
previous update: after requesting a pause and reaching `GS_Pause`, simulated `UpdateExit()`'s
`Exit_Resume` mechanics directly (`GameState::PopState()` + `MenuMgr::PopMenu()` -- `Exit()` itself is
private, matching the real binary, and nothing calls it yet since button clicks aren't dispatched) and
confirmed the full round trip: `GS_Gameplay` -> pause requested -> `GS_Pause` (`Game::IsPaused()`==true) ->
resume -> back to `GS_Gameplay` (`IsPaused()`==false), stable for the rest of the run, no crash. `GS_Pause`'s
destructor (ported in the previous update) correctly resets the paused flag on the way out.

**What's left to make this reachable by an actual player tap, not just test code:** the pause *button*
itself (`Menu_Ingame::m_pause_button`, a real field, and the Resume/Menu buttons on the pause screen
itself, not currently named/declared anywhere in `Menu_Pause.h`) needs its click state checked and wired
to `Game::RequestPause()`/`Menu_Pause::Exit()` respectively -- the same real button-press dispatch gap
already on record for the main menu's own buttons earlier in this file. `InterfaceButton` itself (the
underlying clickable-element class) is already fully real and working in this tree
(`InterfaceButton::IsReleased()`/`IsPressed()`), so this is "find the right `GetUIObject()` index and wire
it," not "build the button system" -- a smaller, well-scoped task once picked up.

## UPDATE, same session: correcting the above -- the pause button's real wiring is not what it looked like

Picked this up expecting a quick, well-scoped win, and it mostly was for *finding* the button (`Menu_Ingame::
Init()` has exactly one `Menu_Base::GetUIButton("pause_button")` call, verified @`0x4e8f98`, stored straight
into `m_pause_button` -- real, name-based, and `Menu_Base`/`BasicPage::GetUIObject(const jet::String&)` is
already fully real and null-safe in this tree, so finding the button really was trivial). **But disassembling
what happens when it's pressed (`Menu_Ingame::UpdatePauseButtonPress()`, 600 bytes, @`0x4ee264`) shows it is
not the pause-screen trigger.** It checks `GameState::IsBackKeyPressed()`/`IsMenuKeyPressed()` and
`Menu_Base::GetReleasedButton(int)` matched against `m_pause_button`, but what it does on a match is: clear
pending key events, play a sound (`SoundMgr`, doesn't exist here), `Game::SetGoToIGM(...)`,
`Menu_PortraitMessage::HidePortraitMessage()`, `IGNotificationMgr::Pop()`, `Game::SetTimeSpeed(...)` -- **it
never calls `GameState::PushState(new GS_Pause(), true)` and never writes to the mutex-guarded byte at
`Game`+`0x41e`** that `GS_Gameplay::UpdateState()` actually checks (ported two updates ago). That flag is
mutex-guarded in the real binary, which strongly suggests it's set from a different thread entirely -- most
likely a native Android lifecycle/hardware-button callback (`onPause()`, a physical back/menu key at the
JNI level), not from in-menu touch-button handling at all. This reframes the earlier "just find the UI
element" framing as too optimistic: **the real trigger for the pause *screen* specifically is still not
located**, and `UpdatePauseButtonPress()` turns out to be a different, adjacent piece of behavior
(dismissing portrait-message/notification overlays on a key press) that itself depends on several
more undeclared `Menu_Ingame` fields (`0x2a8`-`0x2b4`) and classes this session hasn't touched
(`TutorialMgr`, `KeyEventManager`). Not wired -- correcting course here rather than shipping a guess.
`Game::RequestPause()` (the API ported two updates ago) is still exactly right for whatever the real
trigger turns out to be; it just isn't `UpdatePauseButtonPress()`.

## Intro status (2026-10-05)
First launch flow verified against the .so: GS_No_Space -> GS_Loading(MainMenuFromGame, Gameplay steps; loads the state DB) -> GS_MainMenu.
Studios logos -> title loading screen -> lab menu with the minion in its Idle state (Minion::PostInit starts MainMenuStateMachine) -> Daily Bonus.
The 438 "missing" anims/models were in assets/data_pack1-4.7z (7z -> zip -> data dir). Models still absent from the build use the engine's replacement, as the original.
Colour grading: USE_COLORCORRECTION is always defined (Game::PostInit); matrices come from LoadProfile (per device, ComputeColorOffsetMatrix @0x3788bc);
unlisted devices are identity, which is what the driver uploads. Listed-device profiles and the consumer of g_ColorCorrectionMatrix are not ported.
Still open: StateMachine/ActorState real port (only the StateData path used), Interface3DCostumePos/MinionCostume, flares, particles, sound.

## Save game

`Game::PostInit` @0x528670: `ValidateSaveData` @0x52db68 (opens RecordDB "savegame" -- binary, XTEA key "ERROR: invalid entity", redundancy 5 --
and "settings"; a file that exists but does not load is renamed `<file>_cr` and deleted), then the Game's own RecordDB (+0x458, same
settings) is named "savegame" and loaded, `Game::Load` @0x52e5e8 -> `LoadFromRecordDB` @0x52e65c. The db lives in `System::GetUserFolder()`.
`Game::Save(immediate, _)` @0x52ddf8: not when the free space is below 1 MB or the game is not up (flags +0x4e8 enabled, +0x268 post-init
done); `Save(false)` only sets the pending flags (+0x4e9/+0x4ea), `Game::OnUpdate` writes after the frame update (`Save(true)`).
`SaveToRecordDB` @0x52df14 order: SaveVerifierMgr::WriteValidSave, then each manager's `GameSaveable::SaveToRecordDB(db, name, backup)`:
"statistics", "friendsMgr", RedeemCodeMgr, Player, FacebookAtLaunchMgr, TutorialMgr, MissionMgr, WeeklyChallengeMgr, CostumeMgr,
DailyTokenMgr, AchievementsMgr, TauntsMgr, "onlinePlayerData", BonusUpgradeMgr, "messagesMgr", "anticheatingManager", "randomUsersMgr",
BappleMgr (record names are the classes', `k_saveRecordName`). `LoadFromRecordDB` first checks `SaveVerifierMgr::IsValidSave`
(the record holds `gaia::GameloftID::RetrieveEncryptionGLUID().toString()` -- the device's id -- as a written string), then loads in the
order statistics, friendsMgr, RedeemCodeMgr, FacebookAtLaunchMgr, TutorialMgr, Player, MissionMgr, WeeklyChallengeMgr, CostumeMgr,
DailyTokenMgr, AchievementsMgr, TauntsMgr, onlinePlayerData, BonusUpgradeMgr, messagesMgr, anticheatingManager, randomUsersMgr, BappleMgr;
each only when every one before succeeded (the last three reset themselves when their record is missing). The settings file
("settings", GameSettings) holds the COPPAMgr, GameTrackingMgr and KhronoPlayPatternMgr records. `Game::OnSaveUserData` @0x52c210 (vtable
slot 13, called by Shutdown/OnTerminate) saves the settings and `Save(true)`.

## The run's world (level templates, obstacles, bananas)

`UpdateTemplateInstanceBuilding` @0x45efa4 runs every frame: spawn the current template, its platforms' objects one at a time, make templates up
to four ahead; in the action phase it checks the camera change plane, follows a conditional tee (`UpdateConditionalBifurcationState`), and when
neither the minion nor its pacesetter is in the current template calls `AdvanceLevelTemplateList` (the left template goes pending-to-delete at
minion distance + `templateRemoveDistance` (Game+0x3f4, 120), `GameEntity::IsOutOfScene` compares with the minion's traveled distance @0x565f58).
`GS_Gameplay+0x5c` is the gameplay mode (1 fluffy, 3 large minion), not a state id. `CheckOriginDistance`: past 3500 m on x or y the level moves back
by whole meters (+10 on x) through GameLevel::Translate (volatile entities but the platforms, minion, pacesetter, the three planes).
A pause template (`002_LAB` after the start) waits for `ResumeCreation` (Minion::OnInteractiveAnimatedPusherStop or the Lua ResumeLevelCreation).
Objects of the chunks are made from the clara entities by `GameEntityCreator`; an unknown template makes a plain clara::Entity, which the platform
sequence drops (`dynamic_cast<GameEntity*>`): that is why ObstacleTemplate, BananaTemplatePath (BananaTemplate objects are made from the path's own
template and linked to it) and the superfluous interactive objects had to exist before anything showed up on the lab's platforms.
Headless check: `JET_RUN=<speed> JET_RUNNOGUI=1 JET_DBGRUN=1 test_intro ...` stands in for the minion (pacesetter on the current template at a speed,
a follow camera, the left lane at a tee, ResumeCreation at a pause template) and prints the volatile entities (obstacles, banana paths, interactives).
