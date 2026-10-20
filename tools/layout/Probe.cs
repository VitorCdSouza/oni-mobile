// Test hooks for the phone HUD on the PC (IL2CPP Linux player, see
// tools/scripts/ui_check.sh). With ONI_MOBILE_PROBE=<dir>, once a save has loaded,
// runs the steps of ONI_MOBILE_STEPS (separated by ';') one per second and then
// quits. Steps:
//   shot:<name>     screenshot to <dir>/<name>.png
//   dump:<name>     the UI tree of every root canvas to <dir>/<name>.txt
//   state:<name>    a reference state of the design (MobileHud.ApplyState)
//   sprites:<text>  the names of the loaded sprites that contain text, to <dir>/sprites-<text>.txt
//   wait            nothing, one more second
using System;
using System.IO;
using System.Text;
using UnityEngine;
using UnityEngine.UI;

namespace OniMobile
{
    public static class Probe
    {
        static string dir;
        static string[] steps;
        static int next;
        static float due = -1f;

        public static bool Active
        {
            get { return dir != null; }
        }

        public static void Init()
        {
            dir = Environment.GetEnvironmentVariable("ONI_MOBILE_PROBE");
            if (string.IsNullOrEmpty(dir))
            {
                dir = null;
                return;
            }
            Directory.CreateDirectory(dir);
            string list = Environment.GetEnvironmentVariable("ONI_MOBILE_STEPS") ?? "shot:idle;dump:idle";
            steps = list.Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
        }

        // Called every frame by MobileHud once the game screens exist.
        public static void Tick(MobileHud hud)
        {
            if (dir == null)
                return;
            float now = Time.realtimeSinceStartup;
            if (due < 0f)
            {
                // Let the world and the screens settle after the load.
                due = now + 12f;
                return;
            }
            if (now < due)
                return;
            // A save made with other mods opens a "mod differences" dialog.
            foreach (ConfirmDialogScreen dialog in UnityEngine.Object.FindObjectsByType<ConfirmDialogScreen>(FindObjectsSortMode.None))
                dialog.Deactivate();
            due = now + 1f;
            if (next >= steps.Length)
            {
                Debug.Log("OniMobile probe: done");
                Application.Quit();
                dir = null;
                return;
            }
            string step = steps[next++];
            int colon = step.IndexOf(':');
            string verb = colon < 0 ? step : step.Substring(0, colon);
            string arg = colon < 0 ? "" : step.Substring(colon + 1);
            Debug.Log("OniMobile probe: " + step);
            try
            {
                switch (verb)
                {
                    case "shot":
                        ScreenCapture.CaptureScreenshot(Path.Combine(dir, arg + ".png"));
                        break;
                    case "dump":
                        File.WriteAllText(Path.Combine(dir, arg + ".txt"), DumpCanvases());
                        break;
                    case "sprites":
                        var names = new StringBuilder();
                        foreach (Sprite sprite in Resources.FindObjectsOfTypeAll<Sprite>())
                            if (sprite.name.IndexOf(arg, StringComparison.OrdinalIgnoreCase) >= 0)
                                names.AppendLine(sprite.name);
                        File.WriteAllText(Path.Combine(dir, "sprites-" + arg + ".txt"), names.ToString());
                        break;
                    case "state":
                        hud.ApplyState(arg);
                        break;
                }
            }
            catch (Exception e)
            {
                Debug.LogWarning("OniMobile probe: " + step + " failed: " + e);
            }
        }

        public static string DumpCanvases()
        {
            var text = new StringBuilder();
            text.AppendLine($"screen {Screen.width}x{Screen.height}");
            foreach (Canvas canvas in UnityEngine.Object.FindObjectsByType<Canvas>(FindObjectsInactive.Include, FindObjectsSortMode.None))
            {
                if (!canvas.isRootCanvas)
                    continue;
                text.AppendLine($"=== canvas {canvas.name} mode {canvas.renderMode} order {canvas.sortingOrder} scale {canvas.scaleFactor:F3}");
                Dump(canvas.transform, canvas, 0, text);
            }
            return text.ToString();
        }

        static void Dump(Transform node, Canvas canvas, int depth, StringBuilder text)
        {
            text.Append(' ', depth * 2);
            text.Append(node.gameObject.activeSelf ? "" : "(off) ").Append(node.name);
            var rect = node as RectTransform;
            if (rect != null)
            {
                Rect r = ScreenRect(rect, canvas);
                text.Append($" [{r.xMin:F0},{Screen.height - r.yMax:F0} {r.width:F0}x{r.height:F0}]");
                text.Append($" a({rect.anchorMin.x:G2},{rect.anchorMin.y:G2})-({rect.anchorMax.x:G2},{rect.anchorMax.y:G2})");
                text.Append($" p({rect.pivot.x:G2},{rect.pivot.y:G2}) pos({rect.anchoredPosition.x:F0},{rect.anchoredPosition.y:F0})");
                text.Append($" size({rect.sizeDelta.x:F0},{rect.sizeDelta.y:F0})");
                if (node.localScale != Vector3.one)
                    text.Append($" scale({node.localScale.x:G3})");
            }
            text.Append(" {");
            foreach (Component c in node.GetComponents<Component>())
            {
                if (c == null || c is Transform)
                    continue;
                text.Append(c.GetType().Name).Append(' ');
            }
            text.Append('}');
            var image = node.GetComponent<Image>();
            if (image != null && image.sprite != null)
                text.Append($" sprite={image.sprite.name}/{image.type}/b{image.sprite.border.x:F0}");
            var label = node.GetComponent<TMPro.TMP_Text>();
            if (label != null && !string.IsNullOrEmpty(label.text))
                text.Append(" \"").Append(Shorten(label.text)).Append('"');
            text.AppendLine();
            // Long lists (building grids, table rows) are cut after a few children.
            int shown = 0;
            foreach (Transform child in node)
            {
                if (++shown > 24)
                {
                    text.Append(' ', depth * 2 + 2).AppendLine($"... {node.childCount - 24} more");
                    break;
                }
                Dump(child, canvas, depth + 1, text);
            }
        }

        static string Shorten(string s)
        {
            s = s.Replace('\n', ' ');
            return s.Length > 40 ? s.Substring(0, 40) + "..." : s;
        }

        // Screen rectangle of a UI element, in pixels from the bottom left.
        public static Rect ScreenRect(RectTransform rect, Canvas canvas)
        {
            var corners = new Vector3[4];
            rect.GetWorldCorners(corners);
            Camera camera = canvas != null && canvas.renderMode != RenderMode.ScreenSpaceOverlay ? canvas.worldCamera : null;
            Vector2 a = RectTransformUtility.WorldToScreenPoint(camera, corners[0]);
            Vector2 b = RectTransformUtility.WorldToScreenPoint(camera, corners[2]);
            return Rect.MinMaxRect(Mathf.Min(a.x, b.x), Mathf.Min(a.y, b.y), Mathf.Max(a.x, b.x), Mathf.Max(a.y, b.y));
        }
    }
}
