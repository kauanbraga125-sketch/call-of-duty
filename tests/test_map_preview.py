"""Synthetic-data tests. No Call of Duty commercial assets are distributed."""
import importlib.util
import struct
import tempfile
import unittest
import zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("convert_map", root/"ps4/map_preview/convert_map.py")
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def artificial_bsp(bad_index=False):
    lumps=[b"" for _ in range(33)]
    lumps[0] = b"textures/test/wall".ljust(64,b"\0") + b"\0"*8
    lumps[6] = struct.pack("<HHIHHI",0,65535,0,3,3,0)
    verts=[(0.0,0.0,0.0),(100.0,0.0,0.0),(0.0,100.0,0.0)]
    lumps[7] = b"".join(struct.pack("<10f4B", *(xyz+(0.,0.,0.,0.,0.,0.,1.)),200,200,200,255) for xyz in verts)
    lumps[8] = struct.pack("<HHH",0,1,3 if bad_index else 2)
    lumps[27]=struct.pack("<6f6I",*(0.0,)*6,0,1,0,0,0,0)
    lumps[29]=b'{\n"classname" "info_player_start"\n"origin" "0 0 0"\n"angle" "45"\n}\0'
    buf=bytearray(b"IBSP"+struct.pack("<I",59)+b"\0"*264)
    for i,data in enumerate(lumps):
        struct.pack_into("<II",buf,8+i*8,len(data),len(buf))
        buf.extend(data)
    return bytes(buf)


class ConverterTests(unittest.TestCase):
    def test_read_single_soup_and_mesh(self):
        origin, tris,total=m.convert_bsp(artificial_bsp(),limit=100,radius=1000)
        self.assertEqual(origin,(0,0,0,45))
        self.assertEqual((len(tris),total),(1,1))
        self.assertEqual(tris[0][0], [0.,0.,0.,0.,0.,100/48.,100/48.,0.,0.])
        with tempfile.TemporaryDirectory() as d:
            out=m.write_preview(Path(d)/"preview.c4m",origin,tris)
            payload=out.read_bytes()
            magic,n,x,y,z,yaw=m.HEADER.unpack_from(payload)
            self.assertEqual((magic,n,x,y,z,yaw),(b"C4M1",1,0,0,0,45))
            self.assertEqual(len(payload),m.HEADER.size+m.TRI.size)

    def test_bad_version_rejected(self):
        bad=bytearray(artificial_bsp())
        struct.pack_into("<I",bad,4,46)
        with self.assertRaisesRegex(ValueError,"Wrong BSP version"):
            m.convert_bsp(bytes(bad))

    def test_bad_index_rejected(self):
        with self.assertRaisesRegex(ValueError,"Invalid vertex index"):
            m.convert_bsp(artificial_bsp(bad_index=True))

    def test_pk3_scan_and_extract(self):
        with tempfile.TemporaryDirectory() as d:
            main=Path(d)/"main";main.mkdir()
            with zipfile.ZipFile(main/"pak0.pk3","w",compression=zipfile.ZIP_DEFLATED) as z:
                z.writestr("maps/training.bsp",artificial_bsp())
                z.writestr("maps/mp/mp_carentan.bsp",artificial_bsp())
            records=m.find_maps(main)
            self.assertEqual(len(records),2)
            pick=m.select_map(records,None)
            self.assertEqual(pick[0],"training")
            self.assertEqual(m.read_map(pick), artificial_bsp())

    def test_angles_yaw_second_value(self):
        ents=b'{"classname" "info_player_start" "origin" "1 2 3" "angles" "0 225 0"}'
        self.assertEqual(m.get_spawn(ents,[]),(1.,2.,3.,225.))


if __name__ == "__main__":
    unittest.main()
