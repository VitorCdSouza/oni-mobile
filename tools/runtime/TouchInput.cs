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
//   two fingers               pan with the middle point, pinch to zoom (wheel steps)
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

        // KCanvasScaler's scale when the player has not picked one. The game sizes its UI
        // for a 1080p monitor; on a phone the same pixels are about a third as big.
        // At the 720 line render height (Display.RenderHeight) the game's own screen
        // factor is 0.86, so 1.75 gives about 1.5 times the size 1.5 had at 1080.
        public static float DefaultUIScale()
        {
            return IsAndroid ? 1.75f : 1f;
        }
    }

    public static class TouchInput
    {
        enum Gesture { None, Pending, Drag, Pan, Pinch, Done }

        const int Left = 0, Right = 1, Middle = 2;
        const float LongPressSeconds = 0.5f;
        // Finger distance ratio per zoom step (one wheel notch).
        const float PinchStep = 1.12f;

        static readonly bool[] held = new bool[3];
        static readonly bool[] pressed = new bool[3];
        static readonly bool[] released = new bool[3];
        static int frame = -1;
        static Gesture gesture;
        static Vector2 startPosition;
        static float startTime;
        static float pinchDistance;
        static int scroll;
        static int releaseNextFrame = -1;
        static Vector3 position = new Vector3(Screen.width / 2f, Screen.height / 2f, 0f);
        static Vector3 delta;

        static Func<bool> overUI;
        static Func<bool> selectToolActive;

        public static Vector3 mousePosition
        {
            get
            {
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
            if (count == 0)
                EndTouches();
            else if (count == 1)
                OneFinger(Input.GetTouch(0).position);
            else
                TwoFingers(Input.GetTouch(0).position, Input.GetTouch(1).position);

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
                case Gesture.Pinch:
                    Release(Middle);
                    break;
            }
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
                    gesture = Gesture.Done;
                    break;
            }
        }

        static void TwoFingers(Vector2 a, Vector2 b)
        {
            Vector2 middle = (a + b) / 2f;
            float distance = Mathf.Max((a - b).magnitude, 1f);
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
                    position = middle;
                    if (!held[Middle])
                        Press(Middle);
                    pinchDistance = distance;
                    gesture = Gesture.Pinch;
                    break;
                case Gesture.Pinch:
                    position = middle;
                    float ratio = distance / pinchDistance;
                    if (ratio > PinchStep)
                    {
                        scroll = 1;
                        pinchDistance = distance;
                    }
                    else if (ratio < 1f / PinchStep)
                    {
                        scroll = -1;
                        pinchDistance = distance;
                    }
                    break;
            }
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
