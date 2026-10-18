# ARM diff tests

Build `smdiff.cpp` against core/smath, write its output to `ours.txt`, then run `smdiff.py` next to the 0.0.3 `libdespicablemefree.so` (runs the original code in unicorn through `tools/armcall.py`).
