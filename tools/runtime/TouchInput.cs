// Touch controls for the Android port.
//
// The game reads the mouse through static UnityEngine.Input calls (mouse buttons
// as KeyCode.Mouse0..2, Input.mousePosition, the "Mouse X/Y" and
// "Mouse ScrollWheel" axes). tools/picktypes/PatchAndroid.cs points those calls
// at the methods of the same name and signature below. On Android they report a
// mouse driven by touch gestures; elsewhere they pass straight through to Input.
//
// Gestures:
//   tap                       left click
//   long press                right click (cancels the tool, closes menus)
//   one finger drag           pans the camera (middle button drag), or, when it
//                             starts on the UI or a tool other than Select is
//                             active, drags with the left button (area tools,
//                             scroll lists, sliders)
//   two fingers               pan with the middle point, pinch to zoom
//
// UnityEngine.UI's input module reads the same calls through BaseInput, so it
// sees this mouse too; touchCount reads as 0 there so it does not also handle
// the raw touches.
using System;
using System.Reflection;
using UnityEngine;

namespace OniMobile
{
    public static class Platform
    {
        public static bool IsAndroid
        {
            get { return Application.platform == RuntimePlatform.Android; }
        }

        // The phone layout of the HUD (tools/layout). On the PC, ONI_MOBILE_UI=1 turns it
        // on to check it in the IL2CPP Linux player.
        public static bool MobileUI
        {
            get
            {
                if (mobileUI < 0)
                    mobileUI = IsAndroid || Environment.GetEnvironmentVariable("ONI_MOBILE_UI") == "1" ? 1 : 0;
                return mobileUI == 1;
            }
        }

        static int mobileUI = -1;

        // KCanvasScaler's scale when the player has not picked one. The game sizes its UI
        // for a 1080p monitor; on a phone the same pixels are about a third as big.
        // At the 720 line render height (Display.RenderHeight) the game's own screen
        // factor is 0.86, so 1.75 gives about 1.5 times the size 1.5 had at 1080.
        public static float DefaultUIScale()
        {
            return IsAndroid ? 1.75f : 1f;
        }
    }

    // Starts OniMobile.MobileHud (tools/layout, OniMobile.Layout.dll) on the first input
    // read. That assembly references Assembly-CSharp, which references this one, so it is
    // found by name.
    public static class LayoutLoader
    {
        static bool started;

        public static void Ensure()
        {
            if (started)
                return;
            started = true;
            Type hud = Type.GetType("OniMobile.MobileHud, OniMobile.Layout");
            if (hud == null)
            {
                Debug.LogWarning("OniMobile: OniMobile.Layout not found");
                return;
            }
            var host = new GameObject("OniMobile.MobileHud");
            UnityEngine.Object.DontDestroyOnLoad(host);
            host.AddComponent(hud);
        }
    }

    public static class TouchInput
    {
        enum Gesture { None, Pending, Drag, Pan, Pinch, Done }

        const int Left = 0, Right = 1, Middle = 2;
        const float LongPressSeconds = 0.5f;
        // CameraController.zoomSpeed while pinching: the camera eases toward its target size
        // by zoomSpeed times the frame time, and at this speed it gets there in the frame.
        const float PinchZoomSpeed = 1000f;

        static readonly bool[] held = new bool[3];
        static readonly bool[] pressed = new bool[3];
        static readonly bool[] released = new bool[3];
        static int frame = -1;
        static Gesture gesture;
        static Vector2 startPosition;
        static float startTime;
        static float pinchDistance;
        static int fingerA = -1, fingerB = -1;
        static float wheelLog;
        static int scroll;
        static int releaseNextFrame = -1;
        static Vector3 position = new Vector3(Screen.width / 2f, Screen.height / 2f, 0f);
        static Vector3 delta;

        static Func<bool> overUI;
        static Func<bool> selectToolActive;
        static Func<float, bool> zoomBy;
        static Action endZoom;

        public static Vector3 mousePosition
        {
            get
            {
                LayoutLoader.Ensure();
                if (!Platform.IsAndroid)
                    return Input.mousePosition;
                Update();
                return position;
            }
        }

        public static Vector2 mouseScrollDelta
        {
            get
            {
                if (!Platform.IsAndroid)
                    return Input.mouseScrollDelta;
                Update();
                return new Vector2(0f, scroll);
            }
        }

        public static bool mousePresent
        {
            get { return Platform.IsAndroid || Input.mousePresent; }
        }

        public static bool touchSupported
        {
            get { return !Platform.IsAndroid && Input.touchSupported; }
        }

        public static int touchCount
        {
            get { return Platform.IsAndroid ? 0 : Input.touchCount; }
        }

        public static bool GetMouseButton(int button)
        {
            if (!Platform.IsAndroid)
                return Input.GetMouseButton(button);
            Update();
            return button < 3 && held[button];
        }

        public static bool GetMouseButtonDown(int button)
        {
            if (!Platform.IsAndroid)
                return Input.GetMouseButtonDown(button);
            Update();
            return button < 3 && pressed[button];
        }

        public static bool GetMouseButtonUp(int button)
        {
            if (!Platform.IsAndroid)
                return Input.GetMouseButtonUp(button);
            Update();
            return button < 3 && released[button];
        }

        public static bool GetKey(KeyCode key)
        {
            if (!Platform.IsAndroid || !IsMouse(key))
                return Input.GetKey(key);
            return GetMouseButton(key - KeyCode.Mouse0);
        }

        public static bool GetKeyDown(KeyCode key)
        {
            if (!Platform.IsAndroid || !IsMouse(key))
                return Input.GetKeyDown(key);
            return GetMouseButtonDown(key - KeyCode.Mouse0);
        }

        public static bool GetKeyUp(KeyCode key)
        {
            if (!Platform.IsAndroid || !IsMouse(key))
                return Input.GetKeyUp(key);
            return GetMouseButtonUp(key - KeyCode.Mouse0);
        }

        public static float GetAxis(string axis)
        {
            float value;
            return Platform.IsAndroid && MouseAxis(axis, out value) ? value : Input.GetAxis(axis);
        }

        public static float GetAxisRaw(string axis)
        {
            float value;
            return Platform.IsAndroid && MouseAxis(axis, out value) ? value : Input.GetAxisRaw(axis);
        }

        // UnityEngine.UI's StandaloneInputModule.ProcessMouseEvent raycasts the UI under
        // the pointer every frame, for hover. PatchAndroid makes it return early while
        // this is true: no finger down and no button or wheel change this frame, so the
        // pointer has not moved. Every IdleRaycastFrames frames it still runs, to follow
        // UI that opens or closes under the pointer.
        const int IdleRaycastFrames = 10;

        public static bool PointerIdle()
        {
            if (!Platform.IsAndroid)
                return false;
            Update();
            if (gesture != Gesture.None || releaseNextFrame >= 0 || scroll != 0 || delta != Vector3.zero)
                return false;
            for (int i = 0; i < 3; i++)
                if (held[i] || pressed[i] || released[i])
                    return false;
            return Time.frameCount % IdleRaycastFrames != 0;
        }

        static bool IsMouse(KeyCode key)
        {
            return key >= KeyCode.Mouse0 && key <= KeyCode.Mouse6;
        }

        // Unity scales mouse deltas by the axis sensitivity, 0.1 by default.
        static bool MouseAxis(string axis, out float value)
        {
            switch (axis)
            {
                case "Mouse X": Update(); value = delta.x * 0.1f; return true;
                case "Mouse Y": Update(); value = delta.y * 0.1f; return true;
                case "Mouse ScrollWheel": Update(); value = scroll * 0.1f; return true;
                default: value = 0f; return false;
            }
        }

        static void Press(int button)
        {
            held[button] = true;
            pressed[button] = true;
        }

        static void Release(int button)
        {
            if (!held[button])
                return;
            held[button] = false;
            released[button] = true;
        }

        static void Click(int button)
        {
            Press(button);
            releaseNextFrame = button;
        }

        // Runs once per frame, on the first input read of that frame.
        static void Update()
        {
            if (frame == Time.frameCount)
                return;
            frame = Time.frameCount;
            FrameStats.Tick();
            Array.Clear(pressed, 0, 3);
            Array.Clear(released, 0, 3);
            scroll = 0;
            if (releaseNextFrame >= 0)
            {
                Release(releaseNextFrame);
                releaseNextFrame = -1;
            }
            Vector3 previous = position;

            int count = Input.touchCount;
            if (!Application.isFocused)
                CancelTouches();
            else if (count == 0)
                EndTouches();
            else if (count == 1)
                OneFinger(Input.GetTouch(0).position);
            else
                TwoFingers();

            delta = position - previous;
        }

        static void EndTouches()
        {
            switch (gesture)
            {
                case Gesture.Pending:
                    Click(Left);
                    break;
                case Gesture.Drag:
                    Release(Left);
                    break;
                case Gesture.Pan:
                    Release(Middle);
                    break;
                case Gesture.Pinch:
                    Release(Middle);
                    EndZoom();
                    break;
            }
            gesture = Gesture.None;
        }

        // The app lost focus (notification shade, app switch) with fingers down: let go of
        // every button without the click a lifted finger would give.
        static void CancelTouches()
        {
            if (gesture == Gesture.None)
                return;
            Release(Left);
            Release(Middle);
            if (gesture == Gesture.Pinch)
                EndZoom();
            gesture = Gesture.None;
        }

        static void OneFinger(Vector2 finger)
        {
            switch (gesture)
            {
                case Gesture.None:
                    gesture = Gesture.Pending;
                    startPosition = finger;
                    startTime = Time.unscaledTime;
                    position = finger;
                    break;
                case Gesture.Pending:
                    if ((finger - startPosition).magnitude > DragSlop())
                    {
                        // Press where the finger went down, then follow it from the next frame.
                        position = startPosition;
                        if (OverUI() || !SelectToolActive())
                        {
                            gesture = Gesture.Drag;
                            Press(Left);
                        }
                        else
                        {
                            gesture = Gesture.Pan;
                            Press(Middle);
                        }
                    }
                    else if (Time.unscaledTime - startTime > LongPressSeconds)
                    {
                        gesture = Gesture.Done;
                        position = finger;
                        Click(Right);
                    }
                    break;
                case Gesture.Drag:
                case Gesture.Pan:
                    position = finger;
                    break;
                case Gesture.Pinch:
                    // One of the two fingers lifted: stop, and wait for the other.
                    Release(Middle);
                    EndZoom();
                    gesture = Gesture.Done;
                    break;
            }
        }

        // The pinch follows the same two fingers by id: with a third finger down, or one
        // lifting while another lands in the same frame, the touch indexes change.
        static void TwoFingers()
        {
            Touch a, b;
            bool same = true;
            if (!FindFinger(fingerA, out a) || !FindFinger(fingerB, out b))
            {
                a = Input.GetTouch(0);
                b = Input.GetTouch(1);
                fingerA = a.fingerId;
                fingerB = b.fingerId;
                same = false;
            }
            Vector2 middle = (a.position + b.position) / 2f;
            float distance = Mathf.Max((a.position - b.position).magnitude, 1f);
            switch (gesture)
            {
                case Gesture.Drag:
                    // An area drag ends where it is; the second finger starts a new gesture.
                    Release(Left);
                    gesture = Gesture.Done;
                    break;
                case Gesture.None:
                case Gesture.Pending:
                case Gesture.Pan:
                    StartPinch(middle, distance);
                    break;
                case Gesture.Pinch:
                    if (!same)
                    {
                        StartPinch(middle, distance);
                        break;
                    }
                    position = middle;
                    if (!held[Middle])
                        Press(Middle);
                    if (distance != pinchDistance)
                        ZoomBy(distance / pinchDistance);
                    pinchDistance = distance;
                    break;
            }
        }

        // The camera pans by the pointer's distance from where the middle button went down.
        // A pan under one finger, or a pinch whose fingers changed, holds it somewhere else:
        // it is let go where it is and pressed again under the middle of the fingers on the
        // next frame, so the camera does not jump to the new point.
        static void StartPinch(Vector2 middle, float distance)
        {
            if (held[Middle])
                Release(Middle);
            else
            {
                position = middle;
                Press(Middle);
            }
            pinchDistance = distance;
            wheelLog = 0f;
            gesture = Gesture.Pinch;
        }

        static bool FindFinger(int id, out Touch touch)
        {
            for (int i = Input.touchCount - 1; i >= 0; i--)
            {
                touch = Input.GetTouch(i);
                if (touch.fingerId == id)
                    return true;
            }
            touch = default(Touch);
            return false;
        }

        // Pinch zoom follows the fingers: each frame the camera's target size shrinks by the
        // change in finger distance, and its zoom speed is raised so it gets there at once,
        // around the point under the pointer (the middle of the fingers). Wheel steps, one
        // notch per 12% of finger distance and then eased in, lagged behind the fingers.
        // While a screen holds the camera (DisableUserCameraControl: the starmap, which
        // zooms on its own), the pinch turns into those wheel notches instead.
        const float WheelNotch = 0.11332869f; // ln(1.12)

        static void ZoomBy(float ratio)
        {
            if (zoomBy == null)
                FindCamera();
            if (zoomBy(ratio))
                return;
            wheelLog += Mathf.Log(ratio);
            int notches = (int)(wheelLog / WheelNotch);
            scroll += notches;
            wheelLog -= notches * WheelNotch;
        }

        static void EndZoom()
        {
            if (endZoom == null)
                FindCamera();
            endZoom();
        }

        // CameraController, through reflection (Assembly-CSharp references this assembly).
        static void FindCamera()
        {
            zoomBy = ratio => false;
            endZoom = () => { };
            const BindingFlags member = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance;
            Type type = Type.GetType("CameraController, Assembly-CSharp");
            PropertyInfo instance = type == null ? null : type.GetProperty("Instance", BindingFlags.Public | BindingFlags.Static);
            PropertyInfo target = type == null ? null : type.GetProperty("targetOrthographicSize", member);
            MethodInfo setTarget = target == null ? null : target.GetSetMethod(true);
            FieldInfo speed = type == null ? null : type.GetField("zoomSpeed", member);
            FieldInfo min = type == null ? null : type.GetField("minOrthographicSize", member);
            FieldInfo max = type == null ? null : type.GetField("maxOrthographicSize", member);
            PropertyInfo disabled = type == null ? null : type.GetProperty("DisableUserCameraControl", member);
            if (instance == null || setTarget == null || speed == null || min == null || max == null || disabled == null)
            {
                Debug.LogWarning("OniMobile: CameraController zoom members not found");
                return;
            }
            // Set once per pinch, and given back when it ends.
            float normalSpeed = -1f;
            var args = new object[1];
            zoomBy = ratio =>
            {
                object camera = instance.GetValue(null, null);
                if (camera == null || (bool)disabled.GetValue(camera, null))
                    return false;
                if (normalSpeed < 0f)
                {
                    normalSpeed = (float)speed.GetValue(camera);
                    speed.SetValue(camera, PinchZoomSpeed);
                }
                float size = (float)target.GetValue(camera, null) / ratio;
                size = Mathf.Clamp(size, (float)min.GetValue(camera), (float)max.GetValue(camera));
                args[0] = size;
                setTarget.Invoke(camera, args);
                return true;
            };
            endZoom = () =>
            {
                object camera = instance.GetValue(null, null);
                if (camera != null && normalSpeed >= 0f)
                    speed.SetValue(camera, normalSpeed);
                normalSpeed = -1f;
            };
        }

        static float DragSlop()
        {
            float dpi = Screen.dpi > 0f ? Screen.dpi : 400f;
            return dpi * 0.08f;
        }

        // UnityEngine.EventSystems.EventSystem.current.IsPointerOverGameObject(), through
        // reflection: UnityEngine.UI itself is patched to read input from here.
        static bool OverUI()
        {
            if (overUI == null)
            {
                overUI = () => false;
                Type type = Type.GetType("UnityEngine.EventSystems.EventSystem, UnityEngine.UI");
                PropertyInfo current = type == null ? null : type.GetProperty("current", BindingFlags.Public | BindingFlags.Static);
                MethodInfo over = type == null ? null : type.GetMethod("IsPointerOverGameObject", Type.EmptyTypes);
                if (current != null && over != null)
                    overUI = () =>
                    {
                        object system = current.GetValue(null, null);
                        return system != null && (bool)over.Invoke(system, null);
                    };
            }
            return overUI();
        }

        // PlayerController.Instance.ActiveTool is SelectTool (Assembly-CSharp references this
        // assembly, so it cannot be referenced back).
        static bool SelectToolActive()
        {
            if (selectToolActive == null)
            {
                selectToolActive = () => true;
                Type type = Type.GetType("PlayerController, Assembly-CSharp");
                PropertyInfo instance = type == null ? null : type.GetProperty("Instance", BindingFlags.Public | BindingFlags.Static);
                PropertyInfo tool = type == null ? null : type.GetProperty("ActiveTool", BindingFlags.Public | BindingFlags.Instance);
                if (instance != null && tool != null)
                    selectToolActive = () =>
                    {
                        object controller = instance.GetValue(null, null);
                        object active = controller == null ? null : tool.GetValue(controller, null);
                        return active == null || active.GetType().Name == "SelectTool";
                    };
            }
            return selectToolActive();
        }
    }
}
