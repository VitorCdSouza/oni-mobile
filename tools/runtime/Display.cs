// Render resolution and frame time logging for the Android port.
//
// tools/picktypes/PatchAndroid.cs points the game's Screen.SetResolution calls
// (GraphicsOptionsScreen, at startup and when the options are applied) at
// OniMobile.Display.SetResolution. On Android it renders at RenderHeight lines,
// keeping the screen's aspect, and the system scales the frame up to the panel.
// The phone's GPU fills a 2400x1080 frame through several full screen passes
// (lighting, gases, overlays); 1600x720 is under half the pixels.
using UnityEngine;

namespace OniMobile
{
    public static class Display
    {
        const int RenderHeight = 720;

        public static void SetResolution(int width, int height, FullScreenMode mode, RefreshRate rate)
        {
            if (Platform.IsAndroid && height > RenderHeight)
            {
                width = Mathf.RoundToInt((float)width * RenderHeight / height);
                height = RenderHeight;
            }
            Screen.SetResolution(width, height, mode, rate);
        }
    }

    // Logs the frame rate every LogSeconds on Android (adb logcat -s Unity | grep OniMobile).
    public static class FrameStats
    {
        const float LogSeconds = 10f;

        static float windowStart = -1f;
        static float lastFrame;
        static int frames;
        static float worst;

        // Called once per frame from TouchInput.
        public static void Tick()
        {
            if (!Platform.IsAndroid)
                return;
            float now = Time.realtimeSinceStartup;
            if (windowStart < 0f)
            {
                windowStart = lastFrame = now;
                return;
            }
            worst = Mathf.Max(worst, now - lastFrame);
            lastFrame = now;
            frames++;
            float elapsed = now - windowStart;
            if (elapsed < LogSeconds)
                return;
            Debug.Log($"OniMobile frames: {frames / elapsed:F1} fps, {1000f * elapsed / frames:F1} ms average, "
                + $"{1000f * worst:F0} ms worst, {Screen.width}x{Screen.height}");
            windowStart = now;
            frames = 0;
            worst = 0f;
        }
    }
}
