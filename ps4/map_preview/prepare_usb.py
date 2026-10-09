#!/usr/bin/env python3
"""One-click Windows companion: read the owner's installed CoD1 maps, prepare USB.

Does not upload or redistribute original game files. Uses only local inputs.
"""
from __future__ import annotations

import ctypes
import os
from pathlib import Path
import shutil
import sys
import tempfile
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from convert_map import find_maps, read_map, convert_bsp, write_preview

TITLE = "Call of Duty 1 / PS4 — Preparar pendrive"
PKG_NAME = "IV0000-CODP40001_00-COD1PS4PROTOTYPE.pkg"


def game_candidates():
    """Bounded candidate paths. No full-disk scanning or network access."""
    roots = [Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")),
             Path(os.environ.get("ProgramFiles", r"C:\Program Files")),
             Path("C:/Games"), Path("D:/Games"), Path("D:/"), Path("E:/")]
    names = ["Call Of Duty", "Call of Duty", "Call of Duty 1", "CoD", "CallOfDuty"]
    candidates = []
    for root in roots:
        for name in names:
            candidates.append(root / name / "main")
        candidates.append(root / "Steam/steamapps/common/Call of Duty/main")
        candidates.append(root / "SteamLibrary/steamapps/common/Call of Duty/main")
        candidates.append(root / "steamapps/common/Call of Duty/main")
        candidates.append(root / "GOG Galaxy/Games/Call of Duty/main")
    return candidates


def looks_like_cod_main(path):
    try:
        return path.is_dir() and (
            any(p.suffix.lower() == ".pk3" for p in path.iterdir() if p.is_file())
            or (path/"maps").is_dir()
        )
    except OSError:
        return False


def find_game_main():
    seen = set()
    for path in game_candidates():
        key = str(path).lower()
        if key in seen:
            continue
        seen.add(key)
        if looks_like_cod_main(path):
            return path
    return None


def usb_drives():
    """Windows removable drives (usually USB flash drives)."""
    if os.name != "nt":
        return []
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.GetLogicalDrives.restype = ctypes.c_uint32
    kernel.GetDriveTypeW.argtypes = [ctypes.c_wchar_p]
    kernel.GetDriveTypeW.restype = ctypes.c_uint32
    mask = kernel.GetLogicalDrives()
    drives = []
    for n in range(26):
        if mask & (1 << n):
            root = f"{chr(65+n)}:\\"
            if kernel.GetDriveTypeW(root) == 2:  # DRIVE_REMOVABLE
                drives.append(Path(root))
    return drives


def prefer_record(records):
    if not records:
        return []
    sp = [r for r in records if not r[0].startswith("mp_")]
    if not sp:
        raise ValueError("Nao achei mapas da campanha (apenas mapas multiplayer). "
                         "Selecione uma instalacao contendo mapas Single Player.")
    preferred = ("training", "pathfinder", "burnville", "dawnville", "carride")
    order = {name: i for i, name in enumerate(preferred)}
    return sorted(sp, key=lambda r: (order.get(r[0],100),r[0]))


def convert_first_valid(game_main):
    records = find_maps(game_main)
    candidates = prefer_record(records)
    errors = []
    for record in candidates[:30]:
        try:
            spawn, triangles, _ = convert_bsp(read_map(record),limit=900,radius=2200.0)
            return record[0],spawn,triangles,len(candidates)
        except (OSError, ValueError, EOFError) as exc:
            errors.append(f"{record[0]}: {exc}")
    raise ValueError("Nao consegui converter nenhum mapa de campanha. "
                     "Exemplos de erros:\n" + "\n".join(errors[:4]))


def pkg_in_distribution():
    base = Path(sys.executable if getattr(sys,"frozen",False) else __file__).resolve().parent
    path = base / PKG_NAME
    return path if path.is_file() else None


def prepare(game_main, destination):
    if not looks_like_cod_main(game_main):
        raise ValueError("Pasta do jogo invalida: ela deve ser a pasta main, com .pk3 ou maps.")
    if not destination.is_dir():
        raise ValueError("O pendrive nao esta mais conectado.")
    name, spawn, triangles, candidates = convert_first_valid(game_main)
    # Convert in a temporary OS folder first. Commit to USB only after successful conversion.
    with tempfile.TemporaryDirectory(prefix="cod1ps4_") as temp:
        local = write_preview(Path(temp)/"preview.c4m",spawn,triangles)
        target = destination/"preview.c4m"
        shutil.copy2(local,target)
    pkg = pkg_in_distribution()
    if pkg:
        if pkg.resolve() != (destination/pkg.name).resolve():
            shutil.copy2(pkg,destination/pkg.name)
    return name, len(triangles), candidates, bool(pkg)


def run_app():
    root = tk.Tk()
    root.title(TITLE)
    root.geometry("685x350")
    root.minsize(620,310)
    root.resizable(True,True)
    style = ttk.Style()
    try: style.theme_use("vista")
    except tk.TclError: pass

    frame=ttk.Frame(root,padding=22)
    frame.pack(fill="both",expand=True)
    ttk.Label(frame,text="Preparar mapa original do CoD 1 para PS4",
              font=("Segoe UI",15,"bold")).pack(anchor="w",pady=(0,12))
    description=("Insira um pendrive FAT32 ou exFAT no PC. "
                 "O programa procura sua copia do jogo, escolhe um mapa Single Player "
                 "e coloca o mapa e o PKG no pendrive — sem CMD e sem FTP.")
    ttk.Label(frame,text=description,wraplength=625,justify="left").pack(anchor="w",pady=(0,10))
    installed=find_game_main()
    gamepath=tk.StringVar(value=str(installed) if installed else "Nao encontrado automaticamente")
    ttk.Label(frame,text="Instalacao detectada:",font=("Segoe UI",9,"bold")).pack(anchor="w")
    ttk.Label(frame,textvariable=gamepath,wraplength=615,justify="left").pack(anchor="w",pady=(3,11))
    status=tk.StringVar(value="Pronto. Insira o USB e clique em PREPARAR PENDRIVE.")
    lbl=ttk.Label(frame,textvariable=status,wraplength=610,justify="left")
    lbl.pack(anchor="w",pady=(0,10))

    def click():
        nonlocal installed
        try:
            if not installed:
                messagebox.showinfo(TITLE,"Nao encontrei automaticamente o CoD 1.\n"
                                         "Selecione a pasta 'main' da instalacao original.",parent=root)
                choice=filedialog.askdirectory(title="Selecione a pasta 'main' de Call of Duty 1",parent=root)
                if not choice: return
                installed=Path(choice)
                gamepath.set(str(installed))
            detected=usb_drives()
            if len(detected)==1:
                destination=detected[0]
                if not messagebox.askyesno(TITLE,"Preparar o pendrive "+str(destination)+"?\n\n"
                                                "Os arquivos preview.c4m e o PKG com o mesmo nome serao substituidos, "
                                                "caso ja existam. Outros arquivos permanecem intactos.",parent=root):
                    return
            else:
                messagebox.showinfo(TITLE,"Encontrei "+str(len(detected))+" unidades removiveis.\n"
                                        "Selecione a RAIZ do pendrive na proxima janela (por exemplo E:\\).\n"
                                        "Nao selecione o disco do Windows.",parent=root)
                choice=filedialog.askdirectory(title="Selecione a raiz do pendrive",parent=root)
                if not choice: return
                destination=Path(choice)
                if not messagebox.askyesno(TITLE,"Confirmar gravacao de preview.c4m "
                                                "na pasta:\n"+str(destination)+"?",parent=root):
                    return
            status.set("Procurando e convertendo um mapa da campanha... Aguarde.")
            frame.update_idletasks()
            name,triangles,total,pkg=prepare(installed,destination)
            status.set(f"PRONTO: mapa {name}, {triangles} triangulos, USB {destination}")
            final=(f"Mapa escolhido: {name}\n"
                   f"Triangulos preparados: {triangles}\n"
                   f"Pendrive: {destination}\n"
                   f"preview.c4m: OK\n"
                   f"PKG para instalar: {'COPIADO' if pkg else 'NAO ENCONTRADO NA MESMA PASTA DO EXE'}\n\n"
                   "Agora ejete o pendrive com seguranca e coloque no PS4. "
                   "Ative o GoldHEN, instale o novo PKG pelo Package Installer e abra o aplicativo "
                   "com o pendrive conectado.")
            messagebox.showinfo("Concluido — CoD1 PS4",final,parent=root)
        except Exception as err:
            status.set("Falha: "+str(err).replace("\n"," ")[:250])
            messagebox.showerror("Nao foi possivel preparar o pendrive",str(err),parent=root)

    ttk.Button(frame,text="PREPARAR PENDRIVE AUTOMATICAMENTE",command=click).pack(anchor="w",pady=(2,8),ipadx=15,ipady=8)
    ttk.Label(frame,text="Nenhum arquivo do jogo e enviado para a internet. "
             "Use apenas sua copia legitima do CoD 1.",
             wraplength=610,foreground="#555555").pack(anchor="w",pady=(5,0))
    root.mainloop()


if __name__=="__main__":
    if "--selftest" in sys.argv:
        assert PKG_NAME.endswith(".pkg")
        assert callable(prepare)
        print("One-click companion import OK")
    else:
        run_app()
