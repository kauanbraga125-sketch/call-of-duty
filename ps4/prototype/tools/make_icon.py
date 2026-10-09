"""Generate original 512x512 RGB/24-bit PS4 icon0.png (no alpha)."""
from pathlib import Path
import struct
import sys
import zlib

SIZE = 512

def box(data, x, y, w, h, color):
    for py in range(max(0, y), min(SIZE, y+h)):
        for px in range(max(0, x), min(SIZE, x+w)):
            data[py][px] = color

def pngchunk(tag, data):
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)

def generate(path):
    rows = []
    for y in range(SIZE):
        row = []
        for x in range(SIZE):
            k = int((x + y) / 32)
            row.append((max(5,18+k//5), max(15,33+k//3),max(16,35+k//3)))
        rows.append(row)
    gold=(204,164,95)
    beige=(239,220,184)
    box(rows,42,42,428,8,gold)
    box(rows,42,463,428,8,gold)
    box(rows,42,42,8,429,gold)
    box(rows,462,42,8,429,gold)
    # Geometric COD letters, drawn as an original stencil (not official logo).
    # C
    box(rows,92,162,18,155,beige)
    box(rows,92,162,85,18,beige)
    box(rows,92,299,85,18,beige)
    # O
    box(rows,198,162,18,155,beige)
    box(rows,198,162,86,18,beige)
    box(rows,198,299,86,18,beige)
    box(rows,266,162,18,155,beige)
    # D
    box(rows,306,162,18,155,beige)
    box(rows,306,162,75,18,beige)
    box(rows,306,299,75,18,beige)
    box(rows,363,180,18,119,beige)
    # Crosshair-style emblem and stripe, no commercial assets.
    box(rows,148,372,216,6,gold)
    box(rows,252,348,7,54,gold)
    box(rows,114,423,280,5,gold)
    raw=b''.join(b'\x00'+bytes(v for px in row for v in px) for row in rows)
    hdr=struct.pack(">IIBBBBB", SIZE,SIZE,8,2,0,0,0)
    out=b'\x89PNG\r\n\x1a\n'+pngchunk(b'IHDR',hdr)+pngchunk(b'IDAT',zlib.compress(raw,9))+pngchunk(b'IEND',b'')
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(out)
    print("Original 24-bit RGB icon generated:",path,len(out),"bytes")

if __name__=="__main__":
    generate(sys.argv[1] if len(sys.argv)>1 else "sce_sys/icon0.png")
