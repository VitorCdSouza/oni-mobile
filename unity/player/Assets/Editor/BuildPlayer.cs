using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEditor.Build;
using UnityEditor.Build.Reporting;
using UnityEngine;

// Builds an IL2CPP player whose only content is the game's original managed
// assemblies (copied into Assets/Plugins/Managed). The data files of this
// build are thrown away later and replaced with the converted game data;
// what we keep is the player, libil2cpp.so and global-metadata.dat.
//
// Run with: Unity -batchmode -quit -projectPath <this project>
//   -executeMethod BuildPlayer.Linux   (output: Builds/Linux)
//   -executeMethod BuildPlayer.Android (output: Builds/Android/oni.apk)
public static class BuildPlayer
{
    const string ScenePath = "Assets/Scenes/Empty.unity";
    const string IconPath = "Assets/Icon/icon.png";

    public static void Linux()
    {
        Build(BuildTarget.StandaloneLinux64, NamedBuildTarget.Standalone, "Builds/Linux/OxygenNotIncluded");
    }

    public static void Android()
    {
        PlayerSettings.Android.targetArchitectures = AndroidArchitecture.ARM64;
        PlayerSettings.Android.minSdkVersion = AndroidSdkVersions.AndroidApiLevel26;
        PlayerSettings.SetGraphicsAPIs(BuildTarget.Android, new[] { UnityEngine.Rendering.GraphicsDeviceType.OpenGLES3 });
        EditorUserBuildSettings.buildAppBundle = false;
        PlayerSettings.defaultInterfaceOrientation = UIOrientation.AutoRotation;
        PlayerSettings.allowedAutorotateToPortrait = false;
        PlayerSettings.allowedAutorotateToPortraitUpsideDown = false;
        PlayerSettings.allowedAutorotateToLandscapeLeft = true;
        PlayerSettings.allowedAutorotateToLandscapeRight = true;
        // The PC game's icon, copied in by tools/scripts/setup_android_plugins.sh.
        var icon = AssetDatabase.LoadAssetAtPath<Texture2D>(IconPath);
        if (icon != null)
            PlayerSettings.SetIcons(NamedBuildTarget.Unknown, new[] { icon }, IconKind.Application);
        Build(BuildTarget.Android, NamedBuildTarget.Android, "Builds/Android/oni.apk");
    }

    // Builds Assets/ShaderProbe into one bundle per graphics API, to compare how
    // Unity serializes the same shader for GLES3 and for GLCore.
    public static void ShaderProbe()
    {
        PlayerSettings.SetGraphicsAPIs(BuildTarget.Android, new[] { UnityEngine.Rendering.GraphicsDeviceType.OpenGLES3 });
        PlayerSettings.SetUseDefaultGraphicsAPIs(BuildTarget.StandaloneLinux64, false);
        PlayerSettings.SetGraphicsAPIs(BuildTarget.StandaloneLinux64, new[] { UnityEngine.Rendering.GraphicsDeviceType.OpenGLCore });
        var bundle = new AssetBundleBuild
        {
            assetBundleName = "probe",
            assetNames = new[] { "Assets/ShaderProbe/Probe.shader" },
        };
        foreach (var (target, dir) in new[] { (BuildTarget.Android, "Builds/Probe/Android"), (BuildTarget.StandaloneLinux64, "Builds/Probe/Linux") })
        {
            Directory.CreateDirectory(dir);
            BuildPipeline.BuildAssetBundles(dir, new[] { bundle }, BuildAssetBundleOptions.UncompressedAssetBundle, target);
        }
        EditorApplication.Exit(0);
    }

    static void Build(BuildTarget target, NamedBuildTarget named, string output)
    {
        EnsureEmptyScene();
        PlayerSettings.companyName = "Klei";
        PlayerSettings.productName = "Oxygen Not Included";
        PlayerSettings.SetApplicationIdentifier(named, "local.oni.mobile");
        PlayerSettings.SetScriptingBackend(named, ScriptingImplementation.IL2CPP);
        PlayerSettings.SetApiCompatibilityLevel(named, ApiCompatibilityLevel.NET_Standard);
        // IL2CPP cannot run with stripping disabled; link.xml keeps every type.
        PlayerSettings.SetManagedStrippingLevel(named, ManagedStrippingLevel.Minimal);
        // OptimizeSize shares one generic body across value types too, so every List<T>,
        // Dictionary<K, V> or KCompactedVector<T> of a struct copies its elements with memmove
        // and calls through RuntimeInvoker. A late game save spent 10-15% of the main thread
        // there. OptimizeSpeed generates a body per value type instantiation.
        PlayerSettings.SetIl2CppCodeGeneration(named, Il2CppCodeGeneration.OptimizeSpeed);
        // The game scenes are not in this build, so module usage cannot be inferred from them.
        PlayerSettings.stripEngineCode = false;

        Directory.CreateDirectory(Path.GetDirectoryName(output));
        var options = new BuildPlayerOptions
        {
            scenes = new[] { ScenePath },
            locationPathName = output,
            target = target,
            options = BuildOptions.None,
        };
        BuildReport report = BuildPipeline.BuildPlayer(options);
        BuildSummary summary = report.summary;
        Debug.Log($"[BuildPlayer] {target}: {summary.result}, {summary.totalErrors} errors, {summary.totalSize} bytes, {summary.totalTime}");
        foreach (var step in report.steps)
            foreach (var message in step.messages.Where(m => m.type == LogType.Error || m.type == LogType.Exception))
                Debug.Log($"[BuildPlayer] {step.name}: {message.content}");
        EditorApplication.Exit(summary.result == BuildResult.Succeeded ? 0 : 1);
    }

    static void EnsureEmptyScene()
    {
        if (File.Exists(ScenePath))
            return;
        Directory.CreateDirectory(Path.GetDirectoryName(ScenePath));
        var scene = UnityEditor.SceneManagement.EditorSceneManager.NewScene(
            UnityEditor.SceneManagement.NewSceneSetup.EmptyScene,
            UnityEditor.SceneManagement.NewSceneMode.Single);
        UnityEditor.SceneManagement.EditorSceneManager.SaveScene(scene, ScenePath);
    }
}
