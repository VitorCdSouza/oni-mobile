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

    // Sets the frame rate cap on Android and logs the frame rate every LogSeconds
    // (adb logcat -s Unity | grep OniMobile). With the frame timing stats on
    // (convert_data.py sets enableFrameTimingStats) the log also splits the frame
    // time into main thread, render thread and GPU, and the main thread's wait
    // for the frame cap: a frame whose main thread time is close to the whole frame
    // is CPU bound, one with a long GPU time or present wait is not.
    public static class FrameStats
    {
        const float LogSeconds = 10f;
        // Unity's default on Android is 30, which the game never changes.
        const int TargetFrameRate = 60;

        static float windowStart = -1f;
        static float lastFrame;
        static int frames;
        static float worst;
        static readonly FrameTiming[] timing = new FrameTiming[1];
        static int timed;
        static double mainThread, renderThread, gpu, presentWait;

        // Called once per frame from TouchInput.
        public static void Tick()
        {
            if (!Platform.IsAndroid)
                return;
            float now = Time.realtimeSinceStartup;
            if (windowStart < 0f)
            {
                Application.targetFrameRate = TargetFrameRate;
                windowStart = lastFrame = now;
                return;
            }
            worst = Mathf.Max(worst, now - lastFrame);
            lastFrame = now;
            frames++;
            FrameTimingManager.CaptureFrameTimings();
            if (FrameTimingManager.GetLatestTimings(1, timing) == 1)
            {
                mainThread += timing[0].cpuMainThreadFrameTime;
                renderThread += timing[0].cpuRenderThreadFrameTime;
                gpu += timing[0].gpuFrameTime;
                presentWait += timing[0].cpuMainThreadPresentWaitTime;
                timed++;
            }
            float elapsed = now - windowStart;
            if (elapsed < LogSeconds)
                return;
            string split = timed == 0 ? "no frame timings" :
                $"main {mainThread / timed:F1} ms (present wait {presentWait / timed:F1}), "
                + $"render {renderThread / timed:F1} ms, GPU {gpu / timed:F1} ms";
            Debug.Log($"OniMobile frames: {frames / elapsed:F1} fps, {1000f * elapsed / frames:F1} ms average, "
                + $"{1000f * worst:F0} ms worst, {Screen.width}x{Screen.height}, {split}");
            windowStart = now;
            frames = 0;
            worst = 0f;
            timed = 0;
            mainThread = renderThread = gpu = presentWait = 0;
        }
    }
}
