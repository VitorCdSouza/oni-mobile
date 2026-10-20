// The reference states of the phone layout design (docs/ui-mobile-brief.md), reached
// through the game's own calls, for the probe's screenshots (Probe.cs, "state:<name>").
using UnityEngine;

namespace OniMobile
{
    public static class States
    {
        public static void Apply(string name)
        {
            Reset();
            switch (name)
            {
                case "idle":
                    break;
                case "build":
                    PlanScreen.Instance.OpenCategoryByName("Oxygen");
                    break;
                case "place":
                    PlanScreen.Instance.CopyBuildingOrder(Assets.GetBuildingDef("Electrolyzer"), null);
                    break;
                case "dig":
                    ToolMenu.Instance.basicTools[0].toggle.GetComponent<KToggle>().Click();
                    break;
                case "select":
                    Select();
                    break;
                case "prio":
                    ManagementMenu.Instance.TogglePriorities();
                    break;
                case "research":
                    ManagementMenu.Instance.ToggleResearch();
                    break;
                case "overlay":
                    OverlayScreen.Instance.ToggleOverlay(OverlayModes.Oxygen.ID);
                    break;
                default:
                    Debug.LogWarning("OniMobile: no state " + name);
                    break;
            }
        }

        static void Reset()
        {
            ManagementMenu.Instance.CloseAll();
            PlanScreen.Instance.CloseCategoryPanel(false);
            ToolMenu.Instance.ClearSelection();
            PlayerController.Instance.ActivateTool(SelectTool.Instance);
            SelectTool.Instance.Select(null, true);
            if (OverlayScreen.Instance.GetMode() != OverlayModes.None.ID)
                OverlayScreen.Instance.ToggleOverlay(OverlayModes.None.ID);
        }

        // The building closest to the camera, else a duplicant.
        static void Select()
        {
            Vector3 center = CameraController.Instance.transform.position;
            KSelectable best = null;
            float distance = float.MaxValue;
            foreach (BuildingComplete building in Components.BuildingCompletes.Items)
            {
                var selectable = building.GetComponent<KSelectable>();
                if (selectable == null || !selectable.IsSelectable)
                    continue;
                Vector3 offset = building.transform.position - center;
                offset.z = 0f;
                if (offset.sqrMagnitude < distance)
                {
                    distance = offset.sqrMagnitude;
                    best = selectable;
                }
            }
            if (best == null && Components.LiveMinionIdentities.Count > 0)
                best = Components.LiveMinionIdentities.Items[0].GetComponent<KSelectable>();
            if (best != null)
                SelectTool.Instance.Select(best, true);
        }
    }
}
