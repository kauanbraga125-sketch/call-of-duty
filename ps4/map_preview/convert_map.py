#!/usr/bin/env python3
"""Convert a legitimately owned CoD1 IBSP59 map to a small PS4 wireframe preview.

No original game files are included or uploaded. This is a geometry-first
diagnostic preview, NOT a complete BSP engine or a campaign interpreter.
Format research: https://github.com/janost/vcod/blob/main/docs/research/bsp-ibsp59-format.md
Implementation here is independently written in Python stdlib.
"""
import argparse
import math
import re
import struct
import sys
import zipfile
from pathlib import Path

MAX_BSP = 96 * 1024 * 1024
MAX_TRIANGLES = 1200
MAGIC = b"C4M1"
HEADER = struct.Struct("<4sIffff")  # magic, tri count, spawn x/y/z, yaw degrees
TRI = struct.Struct("<9fI")          # PS4 coords (x,y,z)*3, 0xRRGGBB
STARTS = ("info_player_start", "info_player_deathmatch", "mp_deathmatch_spawn",
          "info_player_allied", "info_player_axis", "info_player_intermission")


def find_maps(game_dir):
    game_dir = Path(game_dir)
    if not game_dir.is_dir():
        raise ValueError(f"Game directory does not exist: {game_dir}")
    found = []
    for path in game_dir.rglob("*.bsp"):
        if path.is_file() and path.stat().st_size < MAX_BSP:
            found.append((path.name.lower()[:-4], str(path), None))
    for pak in sorted(game_dir.rglob("*"), key=lambda x: x.name.lower()):
        if not pak.is_file() or pak.suffix.lower() not in (".pk3", ".zip"):
            continue
        try:
            with zipfile.ZipFile(pak) as archive:
                for entry in archive.infolist():
                    name = entry.filename.replace("\\", "/")
                    if (name.lower().endswith(".bsp") and not entry.is_dir()
                            and entry.file_size < MAX_BSP
                            and name.lower().startswith("maps/")):
                        found.append((Path(name).stem.lower(), str(pak), entry.filename))
        except (OSError, zipfile.BadZipFile):
            print(f"Warning: skipping unreadable PK3: {pak}", file=sys.stderr)
    return sorted(found)


def select_map(records, name):
    if not records:
        raise ValueError("No .bsp maps found. Point --game-dir to the installed game's main folder.")
    if name:
        name = Path(name.lower()).stem
        hits = [r for r in records if r[0] == name]
        if not hits:
            raise ValueError(f"Map '{name}' not found. Use --list to see map names.")
        return hits[-1]
    # SP maps are generally 'maps/*.bsp'; avoid mp_* if any campaign maps exist.
    sp = [r for r in records if not r[0].startswith("mp_")]
    return (sp or records)[0]


def read_map(record):
    _, path, entry = record
    if entry is not None:
        with zipfile.ZipFile(path) as pak:
            meta = pak.getinfo(entry)
            if meta.file_size > MAX_BSP:
                raise ValueError("Map exceeds 96 MiB safety limit")
            return pak.read(entry)
    return Path(path).read_bytes()


def read_lumps(data):
    if len(data) < 272 or data[:4] != b"IBSP":
        raise ValueError("Not a CoD1 BSP: header must start with IBSP")
    ver = struct.unpack_from("<I", data, 4)[0]
    if ver != 59:
        raise ValueError(f"Wrong BSP version {ver} (expected CoD1 IBSP 59)")
    lumps = []
    for i in range(33):
        size, offset = struct.unpack_from("<II", data, 8 + i * 8)
        if offset > len(data) or size > len(data) - offset:
            raise ValueError(f"BSP lump {i} is outside the file")
        lumps.append(memoryview(data)[offset:offset + size])
    return lumps


def get_spawn(entities, verts):
    # Find the first explicitly placed player start. Some custom BSPs have none.
    text = bytes(entities).decode("latin1", errors="replace")
    for block in re.findall(r"\{([^{}]*)\}", text, re.S):
        props = {k.lower(): v for k, v in re.findall(r'"([^"]+)"\s*"([^"]*)"', block)}
        if props.get("classname", "").lower() not in STARTS:
            continue
        try:
            pos = tuple(float(v) for v in props["origin"].split())
            if len(pos) != 3 or not all(map(math.isfinite, pos)):
                continue
            angles = props.get("angles", "0 0 0").split()
            yaw = float(props["angle"]) if "angle" in props else float(angles[1] if len(angles)>1 else "0")
            return (*pos, yaw)
        except (ValueError, KeyError):
            continue
    if not verts:
        raise ValueError("No vertices and no valid spawn point")
    xs, ys, zs = zip(*verts)
    # Fallback roughly near geometry center (may be solid; user may need a better map).
    return ((min(xs)+max(xs))/2, (min(ys)+max(ys))/2,
            (min(zs)+max(zs))/2, 0.0)


def convert_bsp(data, limit=900, radius=2200.0):
    lumps = read_lumps(data)
    if len(lumps[0]) % 72 or len(lumps[6]) % 16 or len(lumps[7]) % 44 or len(lumps[8]) % 2:
        raise ValueError("Invalid CoD1 materials/soups/vertices/indices record length")
    if len(lumps[27]) < 48 or len(lumps[27]) % 48:
        raise ValueError("Invalid CoD1 models lump")
    if limit < 1 or limit > MAX_TRIANGLES:
        raise ValueError(f"--max-triangles must be between 1 and {MAX_TRIANGLES}")
    nverts = len(lumps[7]) // 44
    nindices = len(lumps[8]) // 2
    nmaterials = len(lumps[0]) // 72
    verts = [struct.unpack_from("<fff", lumps[7], i*44) for i in range(nverts)]
    ox, oy, oz, yaw = get_spawn(lumps[29], verts)
    # Model 0 owns world geometry. Exclude inline door/trigger brushmodels.
    first_soup, num_soups = struct.unpack_from("<II", lumps[27], 24)
    if first_soup > len(lumps[6])//16 or num_soups > len(lumps[6])//16-first_soup:
        raise ValueError("World model references invalid soups")
    candidates = []
    for s in range(first_soup, first_soup + num_soups):
        mat, lightmap, fv, vc, ic, fi = struct.unpack_from("<HHIHHI", lumps[6], s * 16)
        if mat >= nmaterials or fv + vc > nverts or fi + ic > nindices or ic % 3:
            raise ValueError(f"Invalid triangle soup #{s}")
        matname = bytes(lumps[0][mat*72:mat*72+64]).split(b"\0")[0].lower()
        if b"textures/common/" in matname and any(k in matname for k in
                 (b"clip", b"trigger", b"caulk", b"nodraw", b"sky")):
            continue
        # Stable material tint, NOT a texture: actual textures come later.
        tint = (112 + (mat*31)%105, 132 + (mat*17)%85, 114 + (mat*47)%110)
        color = (tint[0] << 16) | (tint[1] << 8) | tint[2]
        for index in range(fi, fi + ic, 3):
            a, b, c = struct.unpack_from("<HHH", lumps[8], 2*index)
            if max(a,b,c) >= vc:
                raise ValueError(f"Invalid vertex index in soup #{s}")
            triangle = (verts[fv+a], verts[fv+b], verts[fv+c])
            if not all(math.isfinite(v) for p in triangle for v in p):
                continue
            mid = tuple(sum(p[i] for p in triangle)/3 for i in range(3))
            dist2 = (mid[0]-ox)**2 + (mid[1]-oy)**2 + .15*(mid[2]-oz)**2
            if dist2 > radius*radius:
                continue
            candidates.append((dist2, triangle, color))
    # Keep triangles near the spawn point to respect a PS4 CPU rendering budget.
    candidates.sort(key=lambda t: t[0])
    triangles = []
    for _, pts, color in candidates[:limit]:
        converted = []
        for x,y,z in pts:
            converted.extend(((y-oy)/48.0, (z-oz)/48.0, (x-ox)/48.0))
        triangles.append((converted, color))
    if not triangles:
        raise ValueError("No world triangles near spawn. Try --radius 8000")
    return (ox,oy,oz,yaw), triangles, len(candidates)


def write_preview(path, origin, tris):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as f:
        f.write(HEADER.pack(MAGIC, len(tris), *origin))
        for points,color in tris:
            f.write(TRI.pack(*points, color))
    return path


def main(argv=None):
    p = argparse.ArgumentParser(description="CoD1 original map to PS4 geometry preview")
    p.add_argument("--game-dir", required=True, help=r"Original CoD1 main folder, e.g. C:\Program Files (x86)\Call Of Duty\main")
    p.add_argument("--list", action="store_true", help="List BSP names inside local files/PK3 archives")
    p.add_argument("--map", help="Name from --list, e.g. training (optional; defaults to first campaign map)")
    p.add_argument("--output", default="preview.c4m", help="Private generated file to transfer by PS4 FTP")
    p.add_argument("--max-triangles", type=int, default=900)
    p.add_argument("--radius", type=float, default=2200.0, help="Distance from spawn in original CoD units")
    args = p.parse_args(argv)
    records = find_maps(args.game_dir)
    if args.list:
        print(f"{len(records)} map entries found:")
        for name,path,member in records:
            print(f"  {name:26s}  {Path(path).name}/{member or ''}")
        return 0
    chosen = select_map(records,args.map)
    print(f"Reading: {chosen[0]} from {chosen[1]} ({chosen[2] or 'loose BSP'})")
    pos, tris, total = convert_bsp(read_map(chosen),args.max_triangles,args.radius)
    file = write_preview(args.output, pos, tris)
    print(f"Saved {file} ({file.stat().st_size} bytes), {len(tris)} triangles / {total} in radius.")
    print(f"Spawn: {tuple(round(v,2) for v in pos)}")
    print("Copy ONLY preview.c4m into PS4 via FTP: /data/codps4/maps/preview.c4m")
    print("This preview has wireframe geometry only; original textures, game logic and AI are not yet supported.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, OSError, zipfile.BadZipFile) as err:
        print(f"ERROR: {err}", file=sys.stderr)
        sys.exit(1)
