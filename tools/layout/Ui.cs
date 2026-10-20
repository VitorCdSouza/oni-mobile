// Geometry and widgets for the phone HUD.
//
// Positions and sizes are in the design's reference pixels: a 720 line screen
// (1600x720 at the phone's 20:9), whatever the real resolution. Everything lives
// in the game's own ScreenSpaceOverlayCanvas, so the game's UI scale applies to
// the panels moved here and the canvas' raycaster and sorting are shared.
using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace OniMobile
{
    public static class Ui
    {
        public static Canvas Canvas;
        public static RectTransform Root;

        // Canvas units per reference pixel.
        public static float U = 1f;

        // Reference width of the screen (1600 at 20:9).
        public static float Width
        {
            get { return 720f * Screen.width / Screen.height; }
        }

        static TMP_FontAsset font;
        static Material fontMaterial;
        public static Sprite ButtonSprite;
        public static Color ButtonColor = new Color(0.24f, 0.25f, 0.32f, 0.96f);
        public static Color ButtonActiveColor = new Color(0.50f, 0.24f, 0.37f, 1f);
        public static Color PanelColor = new Color(0.16f, 0.17f, 0.22f, 0.94f);
        public static readonly Color TextColor = new Color(0.93f, 0.91f, 0.88f, 1f);

        public static bool Find()
        {
            if (GameScreenManager.Instance == null || GameScreenManager.Instance.ssOverlayCanvas == null)
                return false;
            Canvas = GameScreenManager.Instance.ssOverlayCanvas.GetComponent<Canvas>();
            Root = Canvas.GetComponent<RectTransform>();
            Measure();
            // The look of the game's own buttons: the management bar's toggles.
            if (ManagementMenu.Instance != null)
            {
                foreach (ImageToggleState state in ManagementMenu.Instance.GetComponentsInChildren<ImageToggleState>(true))
                {
                    var image = state.GetComponent<Image>();
                    if (image == null || image.sprite == null || state.name == "OuterGlow" || state.name == "AlertIcon")
                        continue;
                    ButtonColor = state.InactiveColour;
                    ButtonActiveColor = state.ActiveColour;
                    break;
                }
            }
            // The game's rounded box (8 px corners), as on its panels and buttons.
            foreach (Sprite sprite in Resources.FindObjectsOfTypeAll<Sprite>())
            {
                if (sprite.name == "web_rounded_8")
                {
                    ButtonSprite = sprite;
                    break;
                }
            }
            ButtonStyle.Read();
            foreach (LocText text in Root.GetComponentsInChildren<LocText>(true))
            {
                if (text.font == null)
                    continue;
                font = text.font;
                fontMaterial = text.fontSharedMaterial;
                break;
            }
            return true;
        }

        public static void Measure()
        {
            float scale = Canvas.scaleFactor > 0f ? Canvas.scaleFactor : 1f;
            U = Screen.height / 720f / scale;
        }

        public static RectTransform Holder(string name, Transform parent)
        {
            var go = new GameObject(name, typeof(RectTransform));
            go.layer = Root.gameObject.layer;
            var rect = go.GetComponent<RectTransform>();
            rect.SetParent(parent, false);
            rect.anchorMin = rect.anchorMax = new Vector2(0f, 1f);
            rect.pivot = new Vector2(0f, 1f);
            return rect;
        }

        // Places a rectangle by its top left corner, x from the left edge of the screen.
        // With right set, x is the distance from the right edge to the rectangle's right side.
        public static void Place(RectTransform rect, float x, float y, float w, float h, bool right = false)
        {
            var corner = new Vector2(right ? 1f : 0f, 1f);
            if (rect.anchorMin != corner || rect.anchorMax != corner)
            {
                rect.anchorMin = rect.anchorMax = corner;
            }
            rect.pivot = corner;
            rect.anchoredPosition = new Vector2((right ? -x : x) * U, -y * U);
            rect.sizeDelta = new Vector2(w * U, h * U);
        }

        // Moves a rectangle's top left corner to x, y, keeping its size.
        public static void PlaceCorner(RectTransform rect, float x, float y)
        {
            rect.anchorMin = rect.anchorMax = new Vector2(0f, 1f);
            rect.pivot = new Vector2(0f, 1f);
            rect.anchoredPosition = new Vector2(x * U, -y * U);
        }

        public static void Stretch(RectTransform rect)
        {
            rect.anchorMin = Vector2.zero;
            rect.anchorMax = Vector2.one;
            rect.pivot = new Vector2(0.5f, 0.5f);
            rect.anchoredPosition = Vector2.zero;
            rect.sizeDelta = Vector2.zero;
        }

        // Centers a moved game element in its cell, scaled to fit w x h reference pixels.
        public static void Fit(RectTransform rect, float w, float h)
        {
            rect.anchorMin = rect.anchorMax = new Vector2(0.5f, 0.5f);
            rect.pivot = new Vector2(0.5f, 0.5f);
            rect.anchoredPosition = Vector2.zero;
            Vector2 size = rect.rect.size;
            if (size.x < 1f || size.y < 1f)
                return;
            float scale = Mathf.Min(w * U / size.x, h * U / size.y);
            rect.localScale = new Vector3(scale, scale, 1f);
        }

        // Scales a moved game element so its height is h reference pixels.
        public static float ScaleToHeight(RectTransform rect, float h)
        {
            float height = rect.rect.height;
            if (height < 1f)
                return 1f;
            float scale = h * U / height;
            rect.localScale = new Vector3(scale, scale, 1f);
            return scale;
        }

        public static Image Panel(string name, Transform parent)
        {
            var rect = Holder(name, parent);
            var image = rect.gameObject.AddComponent<Image>();
            image.sprite = ButtonSprite;
            image.type = Image.Type.Sliced;
            image.color = PanelColor;
            return image;
        }

        public static TextMeshProUGUI Text(string name, Transform parent, string text, float size)
        {
            var rect = Holder(name, parent);
            var label = rect.gameObject.AddComponent<TextMeshProUGUI>();
            if (font != null)
            {
                label.font = font;
                label.fontSharedMaterial = fontMaterial;
            }
            label.text = text;
            label.fontSize = size * U;
            label.color = TextColor;
            label.alignment = TextAlignmentOptions.Center;
            label.raycastTarget = false;
            label.enableWordWrapping = false;
            label.overflowMode = TextOverflowModes.Overflow;
            return label;
        }

        // A full screen transparent layer that catches taps.
        public static UnityEngine.UI.Button Scrim(string name, Transform parent, System.Action onTap)
        {
            var rect = Holder(name, parent);
            Stretch(rect);
            var image = rect.gameObject.AddComponent<Image>();
            image.color = new Color(0f, 0f, 0f, 0f);
            var button = rect.gameObject.AddComponent<UnityEngine.UI.Button>();
            button.transition = Selectable.Transition.None;
            button.onClick.AddListener(() => onTap());
            return button;
        }

        public static T FindChild<T>(Transform root, string name) where T : Component
        {
            foreach (T c in root.GetComponentsInChildren<T>(true))
                if (c.name == name)
                    return c;
            return null;
        }
    }

    // The look of the game's build category buttons, read from the first one: a light
    // tile with a drop shadow, the icon on it and the label under it.
    public static class ButtonStyle
    {
        public static Sprite Shadow, Normal, Pressed;
        public static Color ShadowColor = new Color(0f, 0f, 0f, 0.5f);
        public static Color NormalColor = Color.white, PressedColor = Color.white;
        public static Vector2 ShadowOffset = new Vector2(8f, -5f);
        public static LocText Label;

        public static void Read()
        {
            if (PlanScreen.Instance == null)
                return;
            KToggle toggle = PlanScreen.Instance.GetComponentInChildren<KToggle>(true);
            if (toggle == null)
                return;
            Transform shadow = toggle.transform.Find("Shadow"), background = toggle.transform.Find("BG");
            if (shadow != null)
            {
                var image = shadow.GetComponent<Image>();
                Shadow = image.sprite;
                ShadowColor = image.color;
                ShadowOffset = ((RectTransform)shadow).anchoredPosition;
            }
            foreach (ImageToggleState state in toggle.GetComponents<ImageToggleState>())
            {
                if (background == null || state.TargetImage == null || state.TargetImage.transform != background)
                    continue;
                Normal = state.InactiveSprite;
                Pressed = state.ActiveSprite != null ? state.ActiveSprite : state.InactiveSprite;
                NormalColor = state.InactiveColour;
                PressedColor = state.ActiveColour;
            }
            if (Normal == null && background != null)
                Normal = Pressed = background.GetComponent<Image>().sprite;
            Label = toggle.GetComponentInChildren<LocText>(true);
        }
    }

    // A button of the phone HUD in the style of the game's build category buttons
    // (ButtonStyle): a game icon or a short text on the tile, and optionally its label
    // under it. The label is also its tip.
    public class HudButton
    {
        public RectTransform Rect;
        readonly Image shadow;
        readonly Image background;
        readonly Image icon;
        readonly TextMeshProUGUI text;
        readonly TextMeshProUGUI caption;
        bool active;

        public HudButton(string name, Transform parent, string sprite, string glyph, string label, System.Action onClick,
            bool showLabel = false)
        {
            Rect = Ui.Holder(name, parent);
            // The button's own image only catches the taps.
            var hit = Rect.gameObject.AddComponent<Image>();
            hit.color = new Color(0f, 0f, 0f, 0f);
            shadow = Ui.Holder("Shadow", Rect).gameObject.AddComponent<Image>();
            shadow.sprite = ButtonStyle.Shadow;
            shadow.color = ButtonStyle.Shadow != null ? ButtonStyle.ShadowColor : new Color(0f, 0f, 0f, 0.35f);
            shadow.raycastTarget = false;
            background = Ui.Holder("BG", Rect).gameObject.AddComponent<Image>();
            background.raycastTarget = false;
            var button = Rect.gameObject.AddComponent<UnityEngine.UI.Button>();
            button.transition = Selectable.Transition.None;
            button.onClick.AddListener(() =>
            {
                KMonoBehaviour.PlaySound(GlobalAssets.GetSound("HUD_Click"));
                onClick();
            });
            Sprite image = string.IsNullOrEmpty(sprite) ? null : Assets.GetSprite(sprite);
            if (image != null)
            {
                icon = Ui.Holder("Icon", Rect).gameObject.AddComponent<Image>();
                icon.sprite = image;
                icon.preserveAspect = true;
                icon.raycastTarget = false;
            }
            else
            {
                text = Ui.Text("Glyph", Rect, glyph ?? label, 26f);
                text.fontStyle = FontStyles.Bold;
            }
            if (showLabel)
            {
                caption = Ui.Text("Label", Rect, label, 16f);
                if (ButtonStyle.Label != null)
                {
                    caption.font = ButtonStyle.Label.font;
                    caption.fontSharedMaterial = ButtonStyle.Label.fontSharedMaterial;
                    caption.color = ButtonStyle.Label.color;
                }
            }
            Tooltip = label;
            Active = false;
            HelpMode.Register(this);
        }

        public string Tooltip;

        public bool Active
        {
            get { return active; }
            set
            {
                active = value;
                background.sprite = value ? ButtonStyle.Pressed : ButtonStyle.Normal;
                background.color = background.sprite == null ? (value ? Ui.ButtonActiveColor : Ui.ButtonColor)
                    : value ? ButtonStyle.PressedColor : ButtonStyle.NormalColor;
            }
        }

        public bool Visible
        {
            get { return Rect.gameObject.activeSelf; }
            set
            {
                if (Rect.gameObject.activeSelf != value)
                    Rect.gameObject.SetActive(value);
            }
        }

        public void Place(float x, float y, float w, float h, bool right = false)
        {
            Ui.Place(Rect, x, y, w, h, right);
            Resize(w, h);
        }

        // Size and contents only, for a button placed by a layout group. The label goes
        // under the tile, outside its rectangle, as on the game's buttons.
        public void Resize(float w, float h)
        {
            Rect.sizeDelta = new Vector2(w * Ui.U, h * Ui.U);
            // The game's drop shadow, about a third as far: at phone size the full one is heavy.
            float shadowX = ButtonStyle.ShadowOffset.x * 0.35f * h / 45f, shadowY = -ButtonStyle.ShadowOffset.y * 0.35f * h / 45f;
            LayoutChild(shadow.rectTransform, shadowX, shadowY, w, h);
            LayoutChild(background.rectTransform, 0f, 0f, w, h);
            float inset = h * 6f / 45f;
            if (icon != null)
                LayoutChild(icon.rectTransform, inset, inset, w - 2f * inset, h - 2f * inset);
            if (text != null)
            {
                LayoutChild(text.rectTransform, 0f, 0f, w, h);
                text.fontSize = Mathf.Min(30f, h * 0.45f) * Ui.U;
            }
            if (caption != null)
            {
                LayoutChild(caption.rectTransform, -16f, h + 1f, w + 32f, 24f);
                caption.fontSize = 18f * Ui.U;
            }
        }

        static void LayoutChild(RectTransform rect, float x, float y, float w, float h)
        {
            Ui.Place(rect, x, y, w, h);
        }
    }
}
