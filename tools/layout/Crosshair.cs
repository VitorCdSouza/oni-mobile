// Crosshair (H6, C2), the touch stand-in for hovering the map: while it is on, the
// pointer rests at a reticle in the middle of the free part of the map, so the game's
// own hover card (HoverTextScreen) shows what is in the cell under it. Dragging the
// camera moves cells under the reticle.
using UnityEngine;
using UnityEngine.UI;

namespace OniMobile
{
    public static class Crosshair
    {
        public static bool On;

        static RectTransform reticle;

        public static void Build(Transform hud)
        {
            reticle = Ui.Holder("C2 reticle", hud);
            foreach (bool vertical in new[] { false, true })
            {
                var bar = Ui.Holder(vertical ? "Vertical" : "Horizontal", reticle);
                var image = bar.gameObject.AddComponent<Image>();
                image.color = new Color(1f, 1f, 1f, 0.85f);
                image.raycastTarget = false;
                bar.anchorMin = bar.anchorMax = bar.pivot = new Vector2(0.5f, 0.5f);
            }
            reticle.gameObject.SetActive(false);
        }

        // left and right: the free part of the map, in reference pixels.
        public static void Layout(float left, float right)
        {
            if (reticle == null)
                return;
            reticle.gameObject.SetActive(On);
            if (!On)
            {
                TouchInput.RestingPointer = null;
                return;
            }
            float x = (left + right) / 2f, y = 400f;
            Ui.Place(reticle, x - 32f, y - 32f, 64f, 64f);
            ((RectTransform)reticle.GetChild(0)).sizeDelta = new Vector2(64f * Ui.U, 3f * Ui.U);
            ((RectTransform)reticle.GetChild(1)).sizeDelta = new Vector2(3f * Ui.U, 64f * Ui.U);
            TouchInput.RestingPointer = new Vector2(x / 720f * Screen.height, (720f - y) / 720f * Screen.height);
        }
    }
}
