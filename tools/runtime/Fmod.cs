// FMOD's Java side on Android.
//
// FMOD reads the banks from the APK (file:///android_asset, see StreamingAssets.cs)
// through Android's AssetManager, which it reaches from org.fmod.FMOD (fmod.jar)
// once that class has the activity. FMODUnity's RuntimeManager.Initialize calls
// org.fmod.FMOD.init under #if UNITY_ANDROID, which the game's PC build of
// FMODUnity.dll compiled out; without it every bank load fails and no event is
// found. tools/picktypes/PatchAndroid.cs puts a call to InitJava at the start of
// RuntimeManager.Initialize.
using System;
using UnityEngine;
using UnityEngine.Android;

namespace OniMobile
{
    public static class Fmod
    {
        static bool done;

        public static void InitJava()
        {
            if (done || !Platform.IsAndroid)
                return;
            done = true;
            try
            {
                using (var fmod = new AndroidJavaClass("org.fmod.FMOD"))
                    fmod.CallStatic("init", AndroidApplication.currentActivity);
                Debug.Log("OniMobile Fmod: org.fmod.FMOD.init done");
            }
            catch (Exception e)
            {
                Debug.LogWarning("OniMobile Fmod: org.fmod.FMOD.init failed: " + e);
            }
        }
    }
}
