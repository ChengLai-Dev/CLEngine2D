"""
Spine 4.0 JSON + .atlas → DragonBones 5.5 JSON 转换器

用途:
  将 Stretchy Studio 导出的 Spine 4.0 格式文件转换为 DragonBones 5.5 格式，
  供 CLEngine2D 的 DBArmatureNode 加载使用。

用法:
  方式 A: 使用 DragonBones 官方 2db 工具 + 本脚本补全纹理 (推荐)
    cd <spine_json_dir>
    2db -t spine -i . -o ./db_output
    python scripts/tools/spine2db.py --make-tex ./db_output/*_ske.json --atlas-dir .

  方式 B: 全部用本脚本处理 (支持 mesh 顶点数据)
    python scripts/tools/spine2db.py <spine.json> [--output <dir>]

输入:
  - Spine 4.0 JSON (Stretchy Studio 导出)
  - Spine .atlas 文件 (可选，自动查找)

输出:
  - xxx_ske.json   (DragonBones 骨骼数据)
  - xxx_tex.json   (DragonBones 纹理图集数据)
  - xxx.png        (纹理图集，复制自输入)
"""

import json
import os
import re
import sys
import shutil
from pathlib import Path


def flip_y(y: float) -> float:
    return -y if y else 0.0


def parse_spine_atlas(atlas_path: str) -> dict:
    """解析 Spine .atlas 文本文件"""
    result = {"pages": []}
    if not atlas_path or not os.path.exists(atlas_path):
        return result
    with open(atlas_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    current_page = None
    current_region = None
    page_pattern = re.compile(r"^(.+\.\w+)$")
    kv_pattern = re.compile(r"^\s+(\w+):\s*(.+)$")

    for line in lines:
        line = line.rstrip("\n\r")
        m = page_pattern.match(line)
        if m and not line.startswith(" ") and not line.startswith("\t"):
            if current_page is not None:
                result["pages"].append(current_page)
            current_page = {
                "imagePath": m.group(1).strip(),
                "width": 0, "height": 0, "format": "RGBA8888",
                "filter": "Linear,Linear", "repeat": "none",
                "regions": {}
            }
            current_region = None
            continue
        if current_page is None:
            continue
        if line.startswith(" ") and not line.startswith("  "):
            name = line.strip()
            current_region = name
            current_page["regions"][name] = {
                "rotate": False, "x": 0, "y": 0,
                "width": 0, "height": 0,
                "origWidth": 0, "origHeight": 0,
                "offsetX": 0, "offsetY": 0, "index": -1
            }
            continue
        m = kv_pattern.match(line)
        if m:
            key, value = m.group(1).strip(), m.group(2).strip()
            if key == "size":
                w, h = value.split(",")
                current_page["width"] = int(w.strip())
                current_page["height"] = int(h.strip())
            elif current_region and current_region in current_page["regions"]:
                r = current_page["regions"][current_region]
                if key == "rotate":
                    r["rotate"] = value.lower() == "true"
                elif key == "xy":
                    x, y = value.split(",")
                    r["x"], r["y"] = int(x.strip()), int(y.strip())
                elif key == "size":
                    w, h = value.split(",")
                    r["width"], r["height"] = int(w.strip()), int(h.strip())
                elif key == "orig":
                    w, h = value.split(",")
                    r["origWidth"], r["origHeight"] = int(w.strip()), int(h.strip())
                elif key == "offset":
                    ox, oy = value.split(",")
                    r["offsetX"], r["offsetY"] = int(ox.strip()), int(oy.strip())
                elif key == "index":
                    r["index"] = int(value)

    if current_page is not None:
        result["pages"].append(current_page)
    return result


def make_tex_json(spine_atlas: dict, atlas_dir: str, output_dir: str) -> str:
    """从解析的 atlas 数据生成 DB 格式 _tex.json，返回输出路径"""
    if not spine_atlas or not spine_atlas.get("pages"):
        return ""
    page = spine_atlas["pages"][0]
    img_path = page["imagePath"]
    # Find actual image
    for p in [os.path.join(atlas_dir, img_path), os.path.join(atlas_dir, os.path.basename(img_path))]:
        if os.path.exists(p):
            img_path = p
            break

    tex_name = Path(img_path).stem if os.path.exists(img_path) else Path(page["imagePath"]).stem
    sub_textures = []
    for region_name, region in page["regions"].items():
        sub = {
            "name": region_name,
            "x": region["x"], "y": region["y"],
            "width": region["width"], "height": region["height"],
        }
        if region.get("origWidth", 0):
            sub["frameWidth"] = region["origWidth"]
        if region.get("origHeight", 0):
            sub["frameHeight"] = region["origHeight"]
        sub_textures.append(sub)

    tex_json = {
        "name": tex_name,
        "imagePath": os.path.basename(img_path) if os.path.exists(img_path) else page["imagePath"],
        "width": page["width"], "height": page["height"],
        "SubTexture": sub_textures
    }

    out_path = os.path.join(output_dir, f"{tex_name}_tex.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(tex_json, f, indent=2, ensure_ascii=False)
    print(f"生成图集: {out_path}")

    # Copy texture
    if os.path.exists(img_path):
        dest = os.path.join(output_dir, os.path.basename(img_path))
        if not os.path.exists(dest):
            shutil.copy2(img_path, dest)
            print(f"复制纹理: {dest}")
    return out_path


# ---- Spine 4.0 → DB 5.5 骨骼转换 ----

SPINE_TYPE_MAP = {
    "region": "image",
    "mesh": "mesh",
    "skinnedmesh": "mesh",
    "boundingbox": "boundingBox",
}


def color_hex_to_db(hex_color: str) -> dict:
    if not hex_color or len(hex_color) < 6:
        return None
    hex_color = hex_color.replace("#", "")
    if len(hex_color) == 8:
        rr, gg, bb, aa = int(hex_color[0:2], 16), int(hex_color[2:4], 16), int(hex_color[4:6], 16), int(hex_color[6:8], 16)
    elif len(hex_color) == 6:
        rr, gg, bb, aa = int(hex_color[0:2], 16), int(hex_color[2:4], 16), int(hex_color[4:6], 16), 255
    else:
        return None
    return {"aM": round(aa / 255 * 100), "rM": round(rr / 255 * 100), "gM": round(gg / 255 * 100), "bM": round(bb / 255 * 100)}


class Spine2DB:
    def __init__(self, spine_json: dict, atlas_data: dict, frame_rate: int = 30, input_filename: str = ""):
        self.spine = spine_json
        self.atlas = atlas_data
        self.frame_rate = frame_rate
        self.name = Path(input_filename).stem if input_filename else "character"
        self.bone_map = {}

    # ── Bones ──
    def convert_bones(self) -> list:
        db_bones = []
        self.bone_map = {}
        for i, bone in enumerate(self.spine.get("bones", [])):
            self.bone_map[bone["name"]] = i
            entry = {"name": bone["name"]}
            if "parent" in bone:
                entry["parent"] = bone["parent"]
            length = bone.get("length", 0)
            if length:
                entry["length"] = length
            tx, ty = bone.get("x", 0), bone.get("y", 0)
            rot = bone.get("rotation", 0)
            scX, scY = bone.get("scaleX", 1), bone.get("scaleY", 1)
            shX, shY = bone.get("shearX", 0), bone.get("shearY", 0)
            if any([tx, ty, rot, scX != 1, scY != 1, shX, shY]):
                tf = {}
                if tx or ty:
                    tf["x"], tf["y"] = tx, flip_y(ty)
                if rot:
                    tf["skX"] = rot
                if scX != 1:
                    tf["scX"] = scX
                if scY != 1:
                    tf["scY"] = scY
                if shX:
                    tf["skX"] = tf.get("skX", 0) + shX
                if shY:
                    tf["skY"] = shY
                entry["transform"] = tf
            db_bones.append(entry)
        return db_bones

    # ── Slots ──
    def convert_slots(self) -> list:
        db_slots = []
        for slot in self.spine.get("slots", []):
            entry = {"name": slot["name"], "parent": slot["bone"]}
            if "color" in slot:
                col = color_hex_to_db(slot["color"])
                if col:
                    entry["color"] = col
            db_slots.append(entry)
        return db_slots

    # ── Skins ──
    def convert_skins(self) -> list:
        spine_skins = self.spine.get("skins", {})
        if isinstance(spine_skins, list):
            return self._convert_skins_old(spine_skins)
        db_slots_map = {}
        for skin_name, skin_data in spine_skins.items():
            for slot_name, slot_attachments in skin_data.items():
                if slot_name not in db_slots_map:
                    db_slots_map[slot_name] = []
                for attach_name, attach_data in slot_attachments.items():
                    if isinstance(attach_data, list):
                        for entry in attach_data:
                            disp = self._convert_attachment(entry, attach_name)
                            if disp:
                                db_slots_map[slot_name].append(disp)
                    else:
                        disp = self._convert_attachment(attach_data, attach_name)
                        if disp:
                            db_slots_map[slot_name].append(disp)
        db_slots_list = [{"name": sn, "display": dl} for sn, dl in db_slots_map.items() if dl]
        return [{"slot": db_slots_list}] if db_slots_list else []

    def _convert_skins_old(self, spine_skins: list) -> list:
        db_slots_map = {}
        for skin in spine_skins:
            attachments = skin.get("attachments", {})
            for slot_name, slot_attachments in attachments.items():
                if slot_name not in db_slots_map:
                    db_slots_map[slot_name] = []
                for attach_name, attach_data in slot_attachments.items():
                    disp = self._convert_attachment(attach_data, attach_name)
                    if disp:
                        db_slots_map[slot_name].append(disp)
        db_slots_list = [{"name": sn, "display": dl} for sn, dl in db_slots_map.items() if dl]
        return [{"slot": db_slots_list}] if db_slots_list else []

    def _convert_attachment(self, data: dict, name: str) -> dict:
        dtype = data.get("type", "region")
        db_type = SPINE_TYPE_MAP.get(dtype, "image")
        display = {"name": name}

        if db_type == "image":
            display["type"] = "image"
            tx, ty = data.get("x", 0), data.get("y", 0)
            rot = data.get("rotation", 0)
            scX, scY = data.get("scaleX", 1), data.get("scaleY", 1)
            w, h = data.get("width", 0), data.get("height", 0)
            if w:
                display["width"] = w
            if h:
                display["height"] = h
            if any([tx, ty, rot, scX != 1, scY != 1]):
                tf = {}
                if tx:
                    tf["x"] = tx
                if ty:
                    tf["y"] = flip_y(ty)
                if rot:
                    tf["skX"] = rot
                if scX != 1:
                    tf["scX"] = scX
                if scY != 1:
                    tf["scY"] = scY
                display["transform"] = tf

        elif db_type == "mesh":
            display["type"] = "mesh"
            display["x"] = data.get("x", 0)
            display["y"] = flip_y(data.get("y", 0))
            display["width"] = data.get("width", 0)
            display["height"] = data.get("height", 0)
            raw_vertices = data.get("vertices", [])
            uvs = data.get("uvs", [])
            triangles = data.get("triangles", [])

            if dtype == "skinnedmesh":
                # Bake weighted vertices: [boneCount, boneIdx, weight, dx, dy, ...]
                baked = []
                i = 0
                while i < len(raw_vertices):
                    bc = int(raw_vertices[i]); i += 1
                    vx, vy = 0.0, 0.0
                    for _ in range(bc):
                        i += 1  # bone index
                        w = float(raw_vertices[i]); i += 1
                        bx = float(raw_vertices[i]); i += 1
                        by = float(raw_vertices[i]); i += 1
                        vx += bx * w; vy += by * w
                    baked.append(vx); baked.append(vy)
                if baked:
                    display["vertices"] = baked
            else:
                if raw_vertices:
                    display["vertices"] = raw_vertices

            if uvs:
                display["uvs"] = uvs
            if triangles:
                display["triangles"] = triangles
            hull = data.get("hull", 0)
            display["verticesCount"] = hull if hull > 0 else (len(display.get("vertices", [])) // 2)

        return display

    # ── Animations ──
    def _get_tween(self, curve):
        if curve is None or curve == "linear":
            return 0.0
        if curve == "stepped":
            return None
        return 0.0

    def convert_animations(self) -> list:
        spine_anims = self.spine.get("animations", {})
        if not spine_anims:
            return []
        db_anims = []
        for anim_name, anim_data in spine_anims.items():
            max_time = self._calc_max_time(anim_data)
            dur = max(1, round(max_time * self.frame_rate))
            db_anim = {"name": anim_name, "duration": dur, "playTimes": 0}

            bones_tl = anim_data.get("bones", {})
            if bones_tl:
                db_bones = []
                for bone_name, timelines in bones_tl.items():
                    entry = {"name": bone_name}
                    if "translate" in timelines:
                        entry["translateFrame"] = self._convert_timeline(timelines["translate"], ["x", "y"])
                    if "rotate" in timelines:
                        entry["rotateFrame"] = self._convert_rotate(timelines["rotate"])
                    if "scale" in timelines:
                        entry["scaleFrame"] = self._convert_timeline(timelines["scale"], ["x", "y"])
                    if entry:
                        db_bones.append(entry)
                if db_bones:
                    db_anim["bone"] = db_bones

            slots_tl = anim_data.get("slots", {})
            if slots_tl:
                db_slots = []
                for slot_name, timelines in slots_tl.items():
                    entry = {"name": slot_name}
                    if "attachment" in timelines:
                        entry["displayFrame"] = self._convert_display(timelines["attachment"])
                    if "color" in timelines:
                        pass
                    if entry:
                        db_slots.append(entry)
                if db_slots:
                    db_anim["slot"] = db_slots

            db_anims.append(db_anim)
        return db_anims

    def _calc_max_time(self, anim_data: dict) -> float:
        max_t = 0.0
        for cat in ("bones", "slots", "deform", "events", "draworder"):
            tl = anim_data.get(cat, {})
            if isinstance(tl, dict):
                for _, v in tl.items():
                    if isinstance(v, dict):
                        for _, kfs in v.items():
                            if isinstance(kfs, list):
                                for kf in kfs:
                                    t = kf.get("time", 0)
                                    if t > max_t:
                                        max_t = t
        return max_t

    def _convert_timeline(self, keyframes: list, fields: list) -> list:
        db_frames = []
        prev_time = 0.0
        for i, kf in enumerate(keyframes):
            time = kf.get("time", 0.0)
            dur = round((time - prev_time) * self.frame_rate) if i > 0 else round(time * self.frame_rate)
            frame = {"duration": max(0, dur)}
            tween = self._get_tween(kf.get("curve"))
            if tween is not None:
                frame["tweenEasing"] = tween
            if "x" in fields and ("x" in kf or "y" in kf):
                frame["x"] = kf.get("x", 0)
                frame["y"] = flip_y(kf.get("y", 0))
            if i == 0 and time == 0:
                frame["duration"] = 0
            db_frames.append(frame)
            prev_time = time
        return db_frames

    def _convert_rotate(self, keyframes: list) -> list:
        db_frames = []
        prev_time = 0.0
        for i, kf in enumerate(keyframes):
            time = kf.get("time", 0.0)
            dur = round((time - prev_time) * self.frame_rate) if i > 0 else round(time * self.frame_rate)
            frame = {"duration": max(0, dur)}
            tween = self._get_tween(kf.get("curve"))
            if tween is not None:
                frame["tweenEasing"] = tween
            angle = kf.get("angle", 0)
            if angle:
                frame["rotate"] = -angle
            if i == 0 and time == 0:
                frame["duration"] = 0
            db_frames.append(frame)
            prev_time = time
        return db_frames

    def _convert_display(self, keyframes: list) -> list:
        db_frames = []
        prev_time = 0.0
        for i, kf in enumerate(keyframes):
            time = kf.get("time", 0.0)
            dur = round((time - prev_time) * self.frame_rate) if i > 0 else round(time * self.frame_rate)
            frame = {"duration": max(0, dur)}
            name = kf.get("name")
            if name is not None and name != "" and name != "null":
                frame["value"] = 0
            else:
                frame["value"] = -1
            if i == 0 and time == 0:
                frame["duration"] = 0
            db_frames.append(frame)
            prev_time = time
        return db_frames

    # ── Build output ──
    def build(self) -> dict:
        arm = {
            "name": self.name,
            "type": "Armature",
            "frameRate": self.frame_rate,
            "bone": self.convert_bones(),
        }
        slots = self.convert_slots()
        if slots:
            arm["slot"] = slots
        skins = self.convert_skins()
        if skins:
            arm["skin"] = skins
        anims = self.convert_animations()
        if anims:
            arm["animation"] = anims
        return {
            "name": self.name,
            "version": "5.5",
            "compatibleVersion": "5.5",
            "frameRate": self.frame_rate,
            "armature": [arm]
        }


# ── CLI ──

def cmd_full_convert(input_json, output_dir, fps):
    """方式 B: 完整转换（骨架 + 图集）"""
    with open(input_json, "r", encoding="utf-8") as f:
        spine_data = json.load(f)

    # Try to find atlas
    base = os.path.splitext(input_json)[0]
    atlas_path = ""
    for ext in [".atlas", ".atlas.txt"]:
        if os.path.exists(base + ext):
            atlas_path = base + ext
            break
    atlas_data = parse_spine_atlas(atlas_path) if atlas_path else {"pages": []}

    os.makedirs(output_dir, exist_ok=True)

    # Convert skeleton
    conv = Spine2DB(spine_data, atlas_data, fps, input_filename=input_json)
    ske = conv.build()
    base_name = Path(input_json).stem
    ske_path = os.path.join(output_dir, f"{base_name}_ske.json")
    with open(ske_path, "w", encoding="utf-8") as f:
        json.dump(ske, f, indent=2, ensure_ascii=False)
    print(f"生成骨骼: {ske_path}")

    # Generate tex.json
    if atlas_path:
        atlas_dir = os.path.dirname(atlas_path)
        make_tex_json(atlas_data, atlas_dir, output_dir)
    else:
        print("警告: 未找到 .atlas 文件，需手动创建 _tex.json")

    print("转换完成!")


def cmd_make_tex(ske_path, atlas_dir):
    """方式 A: 仅补全 _tex.json（2db 已转好 _ske.json 后）"""
    # Find atlas
    atlas_path = ""
    for root, dirs, files in os.walk(atlas_dir):
        for f in files:
            if f.endswith(".atlas") or f.endswith(".atlas.txt"):
                atlas_path = os.path.join(root, f)
                break
        if atlas_path:
            break

    if not atlas_path:
        print("错误: 未找到 .atlas 文件")
        sys.exit(1)

    atlas_data = parse_spine_atlas(atlas_path)
    make_tex_json(atlas_data, os.path.dirname(atlas_path), os.path.dirname(ske_path))


def main():
    if len(sys.argv) < 2:
        print("用法:")
        print("  完整转换:  python spine2db.py <spine.json> [--output <dir>]")
        print("  补图集:    python spine2db.py --make-tex <ske.json> --atlas-dir <dir>")
        sys.exit(1)

    if sys.argv[1] == "--make-tex":
        if len(sys.argv) < 3:
            print("错误: --make-tex 需要指定 _ske.json 路径")
            sys.exit(1)
        ske_path = sys.argv[2]
        atlas_dir = "."
        if "--atlas-dir" in sys.argv:
            idx = sys.argv.index("--atlas-dir")
            if idx + 1 < len(sys.argv):
                atlas_dir = sys.argv[idx + 1]
        cmd_make_tex(ske_path, atlas_dir)
        return

    input_json = sys.argv[1]
    output_dir = "."
    fps = 30
    if "--output" in sys.argv:
        idx = sys.argv.index("--output")
        if idx + 1 < len(sys.argv):
            output_dir = sys.argv[idx + 1]
    if "--fps" in sys.argv:
        idx = sys.argv.index("--fps")
        if idx + 1 < len(sys.argv):
            fps = int(sys.argv[idx + 1])

    cmd_full_convert(input_json, output_dir, fps)


if __name__ == "__main__":
    main()
