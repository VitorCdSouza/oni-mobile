"""Convert every OpenGLCore shader of the game to GLES and check the GLSL with glslang.

Usage: tools/venv/bin/python -I tools/scripts/validate_gles.py <Game_Data> <glslangValidator> <work dir>
"""

import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

import UnityPy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shader_gles  # noqa: E402

FILES = ("resources.assets", "sharedassets0.assets", "sharedassets2.assets", "globalgamemanagers.assets",
         "Resources/unity_builtin_extra",
         "StreamingAssets/expansion1_bundle")
_STAGE = re.compile(r"^#ifdef (VERTEX|FRAGMENT|GEOMETRY|HULL|DOMAIN)\n", re.M)
EXTENSIONS = {"VERTEX": "vert", "FRAGMENT": "frag", "GEOMETRY": "geom", "HULL": "tesc", "DOMAIN": "tese"}


def stages(text):
    """Splits a Unity GLSL text into (stage, source) pairs."""
    marks = list(_STAGE.finditer(text))
    for i, m in enumerate(marks):
        end = marks[i + 1].start() if i + 1 < len(marks) else len(text)
        body = text[m.end():end].rstrip()
        assert body.endswith("#endif"), "unbalanced stage block"
        yield m.group(1), body[:-len("#endif")]


def main():
    data_dir, glslang, work = sys.argv[1:4]
    os.makedirs(work, exist_ok=True)
    jobs = []
    for name in FILES:
        env = UnityPy.load(os.path.join(data_dir, name))
        for obj in env.objects:
            if obj.type.name != "Shader":
                continue
            tree = obj.read_typetree()
            shader = tree["m_ParsedForm"]["m_Name"]
            texts = shader_gles.convert_shader(tree)
            if texts is None:
                continue
            safe = re.sub(r"[^A-Za-z0-9_.-]", "_", shader)
            for i, text in enumerate(texts):
                for stage, src in stages(text):
                    # Unity's own GLES3 output also declares uniform locations, which ES 3.0
                    # rejects; the player turns this define off at load time on such devices.
                    src = src.replace("#define UNITY_SUPPORTS_UNIFORM_LOCATION 1", "#define UNITY_SUPPORTS_UNIFORM_LOCATION 0")
                    path = os.path.join(work, f"{safe}.{obj.path_id}.{i}.{EXTENSIONS[stage]}")
                    with open(path, "w") as f:
                        f.write(src)
                    jobs.append((shader, path))

    def check(job):
        shader, path = job
        r = subprocess.run([glslang, path], capture_output=True, text=True)
        return shader, path, r.returncode, r.stdout

    failed = {}
    with ThreadPoolExecutor(os.cpu_count()) as ex:
        for shader, path, code, out in ex.map(check, jobs):
            if code != 0:
                failed.setdefault(shader, []).append((path, out))
    print(f"{len(jobs)} stages checked, {sum(len(v) for v in failed.values())} failed in {len(failed)} shaders")
    for shader, items in sorted(failed.items()):
        errors = sorted({line for _, out in items for line in out.splitlines() if "ERROR" in line and "compilation errors" not in line})
        print(f"{shader}: {len(items)} stages")
        for line in errors[:5]:
            print("   ", re.sub(r"^ERROR: [^:]+:\d+: ", "", line))


if __name__ == "__main__":
    main()
