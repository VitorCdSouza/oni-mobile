// Tips mode (H9), the touch stand-in for hovering a button: while it is on, a tap on
// any button shows that button's own tooltip in a balloon (C3) instead of pressing
// it. It stays on to read several in a row; the "?" button turns it off.
using System.Collections.Generic;
using System.Text;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace OniMobile
{
    public static class HelpMode
    {
        public static bool On;

        static GameObject scrim;
        static RectTransform balloon;
        static TextMeshProUGUI text;
        static readonly List<RaycastResult> hits = new List<RaycastResult>();
        static readonly Dictionary<GameObject, HudButton> buttons = new Dictionary<GameObject, HudButton>();

        public static void Clear()
        {
            buttons.Clear();
            On = false;
        }

        public static void Register(HudButton button)
        {
            buttons[button.Rect.gameObject] = button;
        }

        public static void Build(Transform hud)
        {
            scrim = Ui.Scrim("Tips scrim", hud, Tap).gameObject;
            balloon = Ui.Panel("C3 tip", hud).rectTransform;
            text = Ui.Text("Text", balloon, "", 18f);
            text.alignment = TextAlignmentOptions.TopLeft;
            text.enableWordWrapping = true;
            balloon.gameObject.SetActive(false);
        }

        public static void Layout()
        {
            if (scrim == null)
                return;
            if (scrim.activeSelf != On)
                scrim.SetActive(On);
            if (!On && balloon.gameObject.activeSelf)
                balloon.gameObject.SetActive(false);
            scrim.transform.SetAsLastSibling();
            balloon.SetAsLastSibling();
        }

        static void Tap()
        {
            TapAt(TouchInput.mousePosition);
        }

        // Also the probe's "tip:x,y" step (reference pixels from the top left).
        public static void TapAt(Vector2 point)
        {
            var data = new PointerEventData(UnityEngine.EventSystems.EventSystem.current) { position = point };
            hits.Clear();
            UnityEngine.EventSystems.EventSystem.current.RaycastAll(data, hits);
            foreach (RaycastResult hit in hits)
            {
                GameObject go = hit.gameObject;
                if (go == scrim || go.transform.IsChildOf(balloon))
                    continue;
                for (Transform t = go.transform; t != null; t = t.parent)
                {
                    HudButton button;
                    if (buttons.TryGetValue(t.gameObject, out button))
                    {
                        if (button.Tooltip == "Tips")
                        {
                            On = false;
                            return;
                        }
                        Show(button.Tooltip, point);
                        return;
                    }
                    var tip = t.GetComponent<ToolTip>();
                    if (tip != null)
                    {
                        string message = Read(tip);
                        if (!string.IsNullOrEmpty(message))
                        {
                            Show(message, point);
                            return;
                        }
                    }
                }
            }
            balloon.gameObject.SetActive(false);
        }

        static string Read(ToolTip tip)
        {
            tip.RebuildDynamicTooltip();
            var message = new StringBuilder();
            for (int i = 0; i < tip.multiStringCount; i++)
            {
                if (message.Length > 0)
                    message.Append('\n');
                message.Append(tip.GetMultiString(i));
            }
            return message.ToString();
        }

        // The balloon right of the tapped point, or left of it near the right edge.
        static void Show(string message, Vector2 screenPoint)
        {
            balloon.gameObject.SetActive(true);
            text.text = message;
            text.fontSize = 18f * Ui.U;
            const float width = 320f;
            float height = Mathf.Clamp(text.GetPreferredValues(message, (width - 24f) * Ui.U, 0f).y / Ui.U + 24f, 48f, 600f);
            float x = screenPoint.x / Screen.height * 720f + 24f;
            float y = 720f - screenPoint.y / Screen.height * 720f;
            if (x + width > Ui.Width - 8f)
                x = x - width - 48f;
            x = Mathf.Clamp(x, 8f, Ui.Width - width - 8f);
            y = Mathf.Clamp(y, 8f, 712f - height);
            Ui.Place(balloon, x, y, width, height);
            Ui.Place(text.rectTransform, 12f, 12f, width - 24f, height - 24f);
        }
    }
}
