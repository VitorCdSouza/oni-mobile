// Render work the game repeats with nothing new to show, cut through tools/picktypes/PatchAndroid.cs.
//
// SimTextures: PropertyTextures uploads the sim's flow, liquid, material and sunlight
// textures (17 bytes a cell) on every frame, but the sim only hands the game new data when
// Game.StepTheSim runs (5 times a second at speed 1). PatchAndroid counts those steps here
// and points the uploads at LoadRawTextureData/Apply below, so each texture goes to the GPU
// once per step.
//
// PostEffects: the overlay camera's KAnimActivePostProcessingEffects and
// CustomActiveScreenPostProcessingEffects copy the whole frame in OnRenderImage even with no
// effect active. They turn themselves off after such a frame and back on when an effect is
// added, so the camera skips the copy (and its intermediate target) in between.
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace OniMobile
{
    public static class SimTextures
    {
        static int step;
        static bool loaded;
        static readonly Dictionary<int, int> uploadedAt = new Dictionary<int, int>();

        public static void Stepped()
        {
            step++;
        }

        public static void LoadRawTextureData(Texture2D texture, IntPtr data, int size)
        {
            int id = texture.GetInstanceID();
            int last;
            loaded = !uploadedAt.TryGetValue(id, out last) || last != step;
            if (!loaded)
                return;
            uploadedAt[id] = step;
            texture.LoadRawTextureData(data, size);
        }

        public static void Apply(Texture2D texture)
        {
            Apply(texture, true, false);
        }

        public static void Apply(Texture2D texture, bool updateMipmaps)
        {
            Apply(texture, updateMipmaps, false);
        }

        public static void Apply(Texture2D texture, bool updateMipmaps, bool makeNoLongerReadable)
        {
            if (loaded)
                texture.Apply(updateMipmaps, makeNoLongerReadable);
            loaded = false;
        }
    }

    public static class PostEffects
    {
        public static void Wake(Behaviour effects)
        {
            effects.enabled = true;
        }

        public static void SleepIfIdle(Behaviour effects, int active)
        {
            if (active == 0)
                effects.enabled = false;
        }

        public static void SleepIfEmpty(Behaviour effects, ICollection active)
        {
            if (active.Count == 0)
                effects.enabled = false;
        }
    }
}
