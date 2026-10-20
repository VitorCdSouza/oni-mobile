// Finds what the GPU spends its time on, on the phone, without rebuilding.
//
// Each time the set of cameras changes it logs every camera (name, depth, target,
// clear flags) and the behaviours on its object, which include the full screen
// effects (OnRenderImage) the game hangs on them: adb logcat -s Unity | grep GfxProbe.
//
// It also reads <persistentDataPath>/gfx-off.txt, one entry per line, and keeps the
// matching behaviours disabled while the line is there:
//   BloomEffect                    that type on every camera object
//   Camera@LightBufferCamera       that type on the object of that name only
// Lines starting with # are ignored. Behaviours it disabled come back on when their
// line goes away. FrameStats then shows the frame and GPU time without them:
//   adb shell "echo BloomEffect > /sdcard/Android/data/local.oni.mobile/files/gfx-off.txt"
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UnityEngine;

namespace OniMobile
{
    public static class GfxProbe
    {
        const int PollFrames = 60;

        static string path;
        static DateTime lastWrite = DateTime.MinValue;
        static string[] rules = new string[0];
        static int cameraCount = -1;
        static readonly List<Behaviour> disabled = new List<Behaviour>();

        // Called once per frame from FrameStats, on Android only.
        public static void Tick()
        {
            if (Time.frameCount % PollFrames != 0)
                return;
            Camera[] cameras = UnityEngine.Object.FindObjectsByType<Camera>(FindObjectsInactive.Include, FindObjectsSortMode.None);
            bool changed = cameras.Length != cameraCount;
            if (changed)
            {
                cameraCount = cameras.Length;
                Dump(cameras);
            }
            if (ReadRules() || changed)
                Apply(cameras);
        }

        static bool ReadRules()
        {
            if (path == null)
                path = Path.Combine(Application.persistentDataPath, "gfx-off.txt");
            DateTime write = File.Exists(path) ? File.GetLastWriteTimeUtc(path) : DateTime.MinValue;
            if (write == lastWrite)
                return false;
            lastWrite = write;
            var lines = new List<string>();
            if (write != DateTime.MinValue)
                foreach (string raw in File.ReadAllLines(path))
                {
                    string line = raw.Trim();
                    if (line.Length > 0 && !line.StartsWith("#"))
                        lines.Add(line);
                }
            rules = lines.ToArray();
            Debug.Log("GfxProbe rules: " + (rules.Length == 0 ? "none" : string.Join(", ", rules)));
            return true;
        }

        static bool Matches(Behaviour behaviour)
        {
            string type = behaviour.GetType().Name;
            foreach (string rule in rules)
            {
                int at = rule.IndexOf('@');
                if (at < 0 ? rule == type : rule.Substring(0, at) == type && rule.Substring(at + 1) == behaviour.gameObject.name)
                    return true;
            }
            return false;
        }

        static void Apply(Camera[] cameras)
        {
            for (int i = disabled.Count - 1; i >= 0; i--)
            {
                Behaviour behaviour = disabled[i];
                if (behaviour == null)
                    disabled.RemoveAt(i);
                else if (!Matches(behaviour))
                {
                    behaviour.enabled = true;
                    disabled.RemoveAt(i);
                    Debug.Log($"GfxProbe on: {behaviour.GetType().Name}@{behaviour.gameObject.name}");
                }
            }
            foreach (Camera camera in cameras)
            {
                foreach (Behaviour behaviour in camera.GetComponents<Behaviour>())
                {
                    if (behaviour.enabled && Matches(behaviour))
                    {
                        behaviour.enabled = false;
                        disabled.Add(behaviour);
                        Debug.Log($"GfxProbe off: {behaviour.GetType().Name}@{behaviour.gameObject.name}");
                    }
                }
            }
        }

        static void Dump(Camera[] cameras)
        {
            Array.Sort(cameras, (a, b) => a.depth.CompareTo(b.depth));
            var text = new StringBuilder($"GfxProbe {cameras.Length} cameras, screen {Screen.width}x{Screen.height}\n");
            foreach (Camera camera in cameras)
            {
                RenderTexture target = camera.targetTexture;
                text.Append($"  {camera.name} depth {camera.depth} {(camera.isActiveAndEnabled ? "on" : "off")} "
                    + $"target {(target == null ? "screen" : $"{target.width}x{target.height} {target.format}")} "
                    + $"clear {camera.clearFlags} mask {camera.cullingMask:x}:");
                foreach (Behaviour behaviour in camera.GetComponents<Behaviour>())
                    if (!(behaviour is Camera))
                        text.Append($" {behaviour.GetType().Name}{(behaviour.enabled ? "" : "(off)")}");
                text.Append('\n');
            }
            Debug.Log(text.ToString());
        }
    }
}
