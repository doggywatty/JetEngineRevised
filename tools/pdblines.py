"""PDB line index from `llvm-pdbutil dump -symbols` / `dump -l`: source file -> [(function, first line, last line)]."""
import bisect, collections, re


def ranges(syms, lines):
    procs = {}
    for m in re.finditer(r"S_[GL]PROC32 \[size = \d+\] `([^`]+)`\n.*?addr = (\d+):(\d+), code size = (\d+)", open(syms).read()):
        procs[(int(m.group(2)), int(m.group(3)))] = (m.group(1), int(m.group(4)))
    starts = sorted(procs)
    idx = collections.defaultdict(lambda: collections.defaultdict(list))
    path = None
    for l in open(lines):
        if "(MD5:" in l:
            path = l.split(" (MD5")[0].strip().lower()
            continue
        for ln, a in re.findall(r"(\d+) ([0-9A-F]{8})", l):
            k = (1, int(a, 16))
            i = bisect.bisect_right(starts, k) - 1
            if i >= 0 and starts[i][0] == 1 and k[1] < starts[i][1] + procs[starts[i]][1]:
                idx[path][procs[starts[i]][0]].append(int(ln))
    return {f: [(n, min(v), max(v)) for n, v in d.items()] for f, d in idx.items()}
