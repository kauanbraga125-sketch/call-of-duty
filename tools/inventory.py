#!/usr/bin/env python3
"""Offline file inventory: never extract or upload commercial game assets."""
import argparse
import json
from collections import Counter
from pathlib import Path

RAR4 = b"Rar!\x1a\x07\x00"
RAR5 = b"Rar!\x1a\x07\x01\x00"


def detect_archive(path):
    with path.open("rb") as fp:
        magic = fp.read(8)
    if magic.startswith(RAR5):
        return "RAR5"
    if magic.startswith(RAR4):
        return "RAR4"
    if magic.startswith(b"PK\x03\x04"):
        return "ZIP"
    return "unknown"


def inventory(path):
    path = Path(path).expanduser().resolve()
    if not path.exists():
        raise FileNotFoundError(path)
    if path.is_file():
        return {"type": "file", "filename": path.name, "bytes": path.stat().st_size,
                "archive_format": detect_archive(path)}
    extensions = Counter()
    packages = []
    count = 0
    bytes_total = 0
    for p in path.rglob("*"):
        if not p.is_file():
            continue
        count += 1
        bytes_total += p.stat().st_size
        ext = p.suffix.lower() or "(sem extensão)"
        extensions[ext] += 1
        if ext in (".pk3", ".iwd", ".pak", ".pk4"):
            packages.append({"name": p.name, "bytes": p.stat().st_size})
    return {"type": "directory", "total_files": count, "total_bytes": bytes_total,
            "extensions": dict(sorted(extensions.items())),
            "game_packages": sorted(packages, key=lambda x: x["name"].lower())}


def main():
    ap = argparse.ArgumentParser(description="Local inventory for CoD 2003 research")
    ap.add_argument("path", help="Directory extracted from your legitimate PC copy, or archive")
    ap.add_argument("--output", help="Save local JSON report")
    args = ap.parse_args()
    report = inventory(args.path)
    data = json.dumps(report, indent=2, ensure_ascii=False)
    if args.output:
        Path(args.output).write_text(data + "\n", encoding="utf-8")
        print("Relatório salvo em", args.output)
    else:
        print(data)


if __name__ == "__main__":
    main()
