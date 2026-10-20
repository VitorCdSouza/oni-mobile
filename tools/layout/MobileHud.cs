// Phone layout of the game's HUD. Started by OniMobile.LayoutLoader
// (tools/runtime/TouchInput.cs) on the first input read.
using UnityEngine;

namespace OniMobile
{
    public class MobileHud : MonoBehaviour
    {
        void Awake()
        {
            Probe.Init();
        }

        void LateUpdate()
        {
            if (Game.Instance == null || SpeedControlScreen.Instance == null)
                return;
            Probe.Tick(this);
        }

        public void ApplyState(string name)
        {
        }
    }
}
