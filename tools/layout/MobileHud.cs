// Phone layout of the game's HUD, after the design in docs/ui-mobile-layout.md
// (1600x720 reference, 20:9 landscape). Started by OniMobile.LayoutLoader
// (tools/runtime/TouchInput.cs) on the first input read; does nothing unless
// OniMobile.Platform.MobileUI.
//
// The game's own panels are moved, not rebuilt: each one is reparented into a
// holder of this HUD and its position, size and scale are set again every frame,
// since the game resets some of them when it refreshes. Only the few new buttons
// (build, exit, rotate) are
// made here. Panels belong to slots that decide what may be open together:
//   slot 0  always on screen: clock, speed, alerts, meters, notifications, rails
//   slot A  work, one at a time: build menu, an active tool, a selection
//   slot B  lookups, one at a time, closed by a tap on the map: more tools
//   slot C  a management screen, which closes A and B
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.UI;

namespace OniMobile
{
    public class MobileHud : MonoBehaviour
    {
        enum Pop { None, MoreTools }

        // The management bar keeps its buttons as icons; of the three small ones at its
        // end only Options stays.
        static readonly string[] DroppedManagement = { "Toggle:REPORTS", "Toggle:DATABASE" };

        // The rail's tools; the rest go to the "more" popover.
        static readonly string[] RailTools = { "DigTool", "CancelTool", "DeconstructTool", "PrioritizeTool", "ClearTool", "MopTool" };

        const float Cell = 72f, Step = 76f, Margin = 8f, Top = 88f;
        // The top row: 64 high, so its buttons match the speed controls.
        const float TopCell = 64f;
        // The cycle clock, and the speed controls right of it.
        const float OverlayCell = 44f;
        // A hammer, the game's build errand icon.
        const string BuildIcon = "icon_errand_build";
        // The diagnostics and resources panels, at the notifications' size.
        const float ColonyScale = 0.8f;
        const float ClockSize = 88f, SpeedX = 56f + ClockSize + 8f;

        Pop pop;
        bool buildMenu;
        bool wasTool, wasSelected, wasManagement, wasPlacing;
        Transform overlaySource;
        ToolMenu.ToolInfo shownTool;
        static FieldInfo managementActive;

        RectTransform hud;
        CanvasGroup left, rail, b1, b2, b3, d1, popover;
        TMPro.TextMeshProUGUI toolName;
        float toolNameUntil;
        RectTransform b1Holder, b2Holder, b3Holder, d1Holder, pHolder, railHolder, notifHolder, legendHolder;
        RectTransform m1Holder, sheetsSource;
        ScrollRect sheetScroll;
        RectTransform paramsHolder, topHolder, overlayRow, popMore, colony;
        CanvasGroup colonyGroup;
        GameObject scrim;

        RectTransform clock, speed, redAlert, meters, notifications;
        RectTransform planGrid, planPanel, productInfo, rootMenu, priority, toolParams, legend;
        readonly List<RectTransform> railCells = new List<RectTransform>();
        readonly List<RectTransform> moreCells = new List<RectTransform>();
        LayoutElement detailsBody;

        RectTransform managementBar, sandbox;
        HudButton build, buildExit, exit, rotate, more;
        CanvasGroup overlayGroup;

        void Awake()
        {
            Probe.Init();
        }

        void LateUpdate()
        {
            if (Game.Instance == null || SpeedControlScreen.Instance == null || PlanScreen.Instance == null
                || ToolMenu.Instance == null || ToolMenu.Instance.basicTools.Count == 0 || ManagementMenu.Instance == null)
                return;
            if (Platform.MobileUI)
            {
                // Built once per game scene: the holder goes with the scene's canvas.
                if (hud == null && Ui.Find())
                    Build();
                if (hud != null)
                {
                    Ui.Measure();
                    Track();
                    Layout();
                }
            }
            Probe.Tick(this);
        }

        public void ApplyState(string name)
        {
            // A KToggle ignores clicks while a CanvasGroup above it is not interactable.
            if (rail != null)
                Show(rail, true);
            States.Apply(name);
            pop = Pop.None;
            buildMenu = name == "build";
            if (name == "popmore")
                pop = Pop.MoreTools;
            else if (name == "buildmenu")
                buildMenu = true;
        }

        // ---- construction -------------------------------------------------------------

        void Build()
        {
            // A new game scene (another save loaded) starts from scratch.
            railCells.Clear();
            moreCells.Clear();
            pop = Pop.None;
            buildMenu = false;
            hud = Ui.Holder("OniMobile.Hud", Ui.Root);
            Ui.Stretch(hud);
            Transform front = Ui.Root.Find("MiddleCenter - InFrontOfEverything");
            if (front != null)
                hud.SetSiblingIndex(front.GetSiblingIndex());

            // Slot A and the rails, under the popovers.
            b1Holder = Ui.Holder("B1 categories", hud);
            b1 = b1Holder.gameObject.AddComponent<CanvasGroup>();
            b2Holder = Ui.Holder("B2 buildings", hud);
            b2 = b2Holder.gameObject.AddComponent<CanvasGroup>();
            b3Holder = Ui.Holder("B3 building", hud);
            b3 = b3Holder.gameObject.AddComponent<CanvasGroup>();
            d1Holder = Ui.Holder("D1 details", hud);
            d1 = d1Holder.gameObject.AddComponent<CanvasGroup>();
            m1Holder = Ui.Holder("M1 management", hud);
            pHolder = Ui.Holder("P priority", hud);
            railHolder = Ui.Holder("H11 tools", hud);
            rail = railHolder.gameObject.AddComponent<CanvasGroup>();
            var leftHolder = Ui.Holder("Left column", hud);
            Ui.Stretch(leftHolder);
            left = leftHolder.gameObject.AddComponent<CanvasGroup>();
            notifHolder = Ui.Holder("H4 notifications", leftHolder);
            legendHolder = Ui.Holder("O2 legend", hud);
            topHolder = Ui.Holder("Top", hud);
            Ui.Stretch(topHolder);

            MoveTop();
            notifications = Move(NotificationScreen.Instance, notifHolder);
            MovePlan();
            MoveTools();
            MoveDetails();
            MoveLegend();
            MoveSheets();

            build = new HudButton("H5 build", hud, BuildIcon, null, "Build", ToggleBuild, true);
            // In build mode the build button gives its place to an exit button.
            buildExit = new HudButton("H5 exit", hud, "icon_action_cancel", "X", "Exit", Exit, true);
            exit = new HudButton("H12 exit", hud, "icon_action_cancel", "X", "Exit", Exit, true);
            rotate = new HudButton("B4 rotate", hud, null, "↻", "Rotate", () =>
            {
                if (BuildTool.Instance != null)
                    BuildTool.Instance.TryRotate();
            }, true);
            toolName = Ui.Text("Tool name", hud, "", 20f);
            more = new HudButton("Q3 more", railHolder, null, "•••", "More", () => Toggle(Pop.MoreTools));

            // Slot B: the scrim that closes a popover on a tap on the map, then the popovers.
            scrim = Ui.Scrim("Scrim", hud, () => pop = Pop.None).gameObject;
            var popHolder = Ui.Holder("Popovers", hud);
            Ui.Stretch(popHolder);
            popover = popHolder.gameObject.AddComponent<CanvasGroup>();
            // O1: the overlay buttons in a row at the bottom right, always at hand.
            overlayRow = Ui.Holder("O1 overlays", hud);
            overlayRow.SetSiblingIndex(scrim.transform.GetSiblingIndex());
            overlayGroup = overlayRow.gameObject.AddComponent<CanvasGroup>();
            var row = overlayRow.gameObject.AddComponent<HorizontalLayoutGroup>();
            row.childControlWidth = false;
            row.childControlHeight = false;
            row.childForceExpandWidth = false;
            row.childForceExpandHeight = false;
            row.childAlignment = TextAnchor.MiddleRight;
            popMore = Grid(Ui.Panel("Q3 more tools", popHolder).rectTransform, 3, Cell, Cell);
            MovePopovers();
        }

        static RectTransform Grid(RectTransform panel, int columns, float w, float h)
        {
            var grid = panel.gameObject.AddComponent<GridLayoutGroup>();
            grid.constraint = GridLayoutGroup.Constraint.FixedColumnCount;
            grid.constraintCount = columns;
            grid.cellSize = new Vector2(w * Ui.U, h * Ui.U);
            grid.spacing = new Vector2(4f * Ui.U, 4f * Ui.U);
            int pad = Mathf.RoundToInt(8f * Ui.U);
            grid.padding = new RectOffset(pad, pad, pad, pad);
            var fitter = panel.gameObject.AddComponent<ContentSizeFitter>();
            fitter.horizontalFit = ContentSizeFitter.FitMode.PreferredSize;
            fitter.verticalFit = ContentSizeFitter.FitMode.PreferredSize;
            return panel;
        }

        static void Hide(Component c)
        {
            if (c == null)
                return;
            var group = c.GetComponent<CanvasGroup>() ?? c.gameObject.AddComponent<CanvasGroup>();
            group.alpha = 0f;
            group.blocksRaycasts = false;
            group.interactable = false;
        }

        static RectTransform Move(Component c, Transform holder)
        {
            if (c == null)
                return null;
            var rect = c.transform as RectTransform;
            rect.SetParent(holder, false);
            return rect;
        }

        // A cell of a rail or grid that holds one game toggle, scaled to fit.
        static RectTransform Wrap(RectTransform toggle, Transform parent, string name)
        {
            var cell = Ui.Holder(name, parent);
            toggle.SetParent(cell, false);
            return cell;
        }

        void MoveTop()
        {
            var date = FindObjectOfType<global::DateTime>();
            clock = Move(date, topHolder);
            speed = Move(SpeedControlScreen.Instance, topHolder);
            Transform topLeft = TopLeftControlScreen.Instance != null ? TopLeftControlScreen.Instance.transform : null;
            if (topLeft != null)
            {
                redAlert = Move(Ui.FindChild<RectTransform>(topLeft, "RedAlertButton"), topHolder);
                meters = Move(MeterScreen.Instance, topHolder);
                // Its width came from the row it was in.
                if (meters != null && meters.GetComponent<ContentSizeFitter>() == null)
                    meters.gameObject.AddComponent<ContentSizeFitter>().horizontalFit = ContentSizeFitter.FitMode.PreferredSize;
                Hide(topLeft);
            }
            Hide(Ui.FindChild<RectTransform>(Ui.Root, "BuildWatermark"));
            // The management bar (Vitals ... Starmap, Options), as icons in the top row.
            managementBar = Move(ManagementMenu.Instance, topHolder);
        }

        void MovePlan()
        {
            planGrid = PlanScreen.Instance.transform as RectTransform;
            var scroll = planGrid.GetComponentInParent<KScrollRect>();
            if (scroll != null)
            {
                scroll.enabled = false;
                // OldPlanScreen: the bottom bar the categories were in.
                Hide(scroll.transform.parent);
            }
            planGrid.SetParent(b1Holder, false);
            planPanel = Move(PlanScreen.Instance.buildingGroupsRoot.parent, b2Holder);
            productInfo = Move(PlanScreen.Instance.ProductInfoScreen, b3Holder);
        }

        void MoveTools()
        {
            var column = railHolder.gameObject.AddComponent<VerticalLayoutGroup>();
            column.childControlWidth = false;
            column.childControlHeight = false;
            column.childForceExpandWidth = false;
            column.childForceExpandHeight = false;
            column.spacing = (Step - Cell) * Ui.U;
            foreach (ToolMenu.ToolCollection collection in ToolMenu.Instance.basicTools)
            {
                if (collection.toggle == null || collection.tools.Count == 0)
                    continue;
                var toggle = collection.toggle.transform as RectTransform;
                if (System.Array.IndexOf(RailTools, collection.tools[0].toolName) >= 0)
                    railCells.Add(Wrap(toggle, railHolder, collection.tools[0].toolName));
                else
                    moreCells.Add(Wrap(toggle, hud, collection.tools[0].toolName));
            }
            priority = Move(ToolMenu.Instance.PriorityScreen, pHolder);
            if (ToolMenu.Instance.toolParameterMenu != null)
            {
                paramsHolder = Ui.Holder("Tool filter", hud);
                toolParams = Move(ToolMenu.Instance.toolParameterMenu, paramsHolder);
            }
        }

        void MoveDetails()
        {
            if (RootMenu.Instance == null)
                return;
            rootMenu = Move(RootMenu.Instance, d1Holder);
            if (DetailsScreen.Instance != null)
            {
                Transform body = DetailsScreen.Instance.transform.Find("Body");
                if (body != null)
                    detailsBody = body.GetComponent<LayoutElement>();
            }
        }

        // The table screens (Vitals, Consumables, Priorities, Schedule, Reports) open in
        // TopRightInfoPanels; here they open under the top row, from the left margin, and
        // the ones wider than the screen pan sideways.
        void MoveSheets()
        {
            var jobs = FindObjectOfType<JobsTableScreen>(true);
            if (jobs == null)
                return;
            sheetsSource = jobs.transform.parent as RectTransform;
            sheetScroll = m1Holder.gameObject.AddComponent<ScrollRect>();
            sheetScroll.viewport = m1Holder;
            sheetScroll.horizontal = true;
            sheetScroll.vertical = false;
            sheetScroll.movementType = ScrollRect.MovementType.Clamped;
            sheetScroll.inertia = true;
            var hit = m1Holder.gameObject.AddComponent<Image>();
            hit.color = new Color(0f, 0f, 0f, 0f);
            hit.raycastTarget = false;
            foreach (Transform screen in sheetsSource.Cast())
                if (screen.GetComponent<KScreen>() != null)
                    screen.SetParent(m1Holder, false);
        }

        void LayoutSheets()
        {
            Ui.Place(m1Holder, 56f, Top, Ui.Width - 56f - Margin, 624f);
            RectTransform open = null;
            foreach (Transform child in m1Holder.Cast())
            {
                if (!child.gameObject.activeInHierarchy || child.localScale.x < 0.01f || child.GetComponent<KScreen>() == null)
                    continue;
                open = (RectTransform)child;
                break;
            }
            if (open == null)
            {
                sheetScroll.content = null;
                return;
            }
            float scale = Mathf.Min(1f, 624f * Ui.U / Mathf.Max(open.rect.height, 1f));
            open.localScale = new Vector3(scale, scale, 1f);
            open.anchorMin = open.anchorMax = new Vector2(0f, 1f);
            open.pivot = new Vector2(0f, 1f);
            if (sheetScroll.content != open)
            {
                sheetScroll.content = open;
                open.anchoredPosition = Vector2.zero;
            }
            else
            {
                open.anchoredPosition = new Vector2(open.anchoredPosition.x, 0f);
            }
        }

        void MoveLegend()
        {
            if (OverlayLegend.Instance != null)
                legend = Move(OverlayLegend.Instance, legendHolder);
        }

        void MovePopovers()
        {
            if (TopLeftControlScreen.Instance != null)
                sandbox = Move(Ui.FindChild<RectTransform>(TopLeftControlScreen.Instance.transform, "SandboxToggle"), topHolder);
            if (OverlayMenu.Instance != null)
            {
                overlaySource = OverlayMenu.Instance.transform.Find("OverlaysBar");
                Adopt(overlaySource, overlayRow, true);
                Hide(OverlayMenu.Instance);
            }
            var diag = FindObjectOfType<ColonyDiagnosticScreen>();
            // Diagnostics and resources, folding panels as on the PC, under the notifications.
            colony = Ui.Holder("Colony", hud);
            var column = colony.gameObject.AddComponent<VerticalLayoutGroup>();
            column.childControlWidth = true;
            column.childControlHeight = true;
            column.childForceExpandWidth = true;
            column.childForceExpandHeight = false;
            column.childAlignment = TextAnchor.UpperLeft;
            colonyGroup = colony.gameObject.AddComponent<CanvasGroup>();
            Move(diag, colony);
            if (PinnedResourcesPanel.Instance != null)
                Move(PinnedResourcesPanel.Instance, colony);
            foreach (RectTransform cell in moreCells)
                cell.SetParent(popMore, false);
        }

        // ---- state ----------------------------------------------------------------------

        bool Placing
        {
            get
            {
                ProductInfoScreen info = PlanScreen.Instance.ProductInfoScreen;
                return info != null && info.gameObject.activeInHierarchy && info.currentDef != null;
            }
        }

        bool Category
        {
            get { return PlanScreen.Instance.ActiveCategoryToggleInfo != null; }
        }

        bool ToolActive
        {
            get
            {
                InterfaceTool tool = PlayerController.Instance.ActiveTool;
                return tool != null && tool != SelectTool.Instance && !Placing;
            }
        }

        bool Selected
        {
            get { return SelectTool.Instance != null && SelectTool.Instance.selected != null; }
        }

        bool ManagementOpen
        {
            get
            {
                if (managementActive == null)
                    managementActive = typeof(ManagementMenu).GetField("activeScreen", BindingFlags.NonPublic | BindingFlags.Instance);
                return managementActive != null && managementActive.GetValue(ManagementMenu.Instance) != null;
            }
        }

        bool BuildOpen
        {
            get { return buildMenu || Category || Placing; }
        }

        // Applies the slot rules to what the game opened or closed since the last frame.
        void Track()
        {
            bool tool = ToolActive, selected = Selected, managementOpen = ManagementOpen;
            if (tool && !wasTool || tool && ToolMenu.Instance.currentlySelectedTool != shownTool)
            {
                // The tool's name, the first line of its tooltip, for 1.8 s over the priority bar.
                shownTool = ToolMenu.Instance.currentlySelectedTool;
                if (shownTool != null)
                {
                    toolName.text = shownTool.text;
                    toolNameUntil = Time.unscaledTime + 1.8f;
                }
            }
            if (tool && !wasTool)
            {
                buildMenu = false;
                pop = Pop.None;
            }
            if (selected && !wasSelected)
                buildMenu = false;
            if (managementOpen && !wasManagement)
            {
                CloseWork();
                pop = Pop.None;
            }
            // Long press (right click) closes one level: a popover, else the build menu.
            if (TouchInput.GetMouseButtonDown(1))
            {
                if (pop != Pop.None)
                    pop = Pop.None;
                else if (buildMenu && !Category && !Placing)
                    buildMenu = false;
            }
            wasTool = tool;
            wasSelected = selected;
            wasManagement = managementOpen;
        }

        void Toggle(Pop which)
        {
            pop = pop == which ? Pop.None : which;
        }

        void ToggleBuild()
        {
            bool open = BuildOpen;
            pop = Pop.None;
            CloseWork();
            buildMenu = !open;
        }

        void CloseWork()
        {
            buildMenu = false;
            if (Placing)
                PlanScreen.Instance.CloseRecipe();
            if (Category)
                PlanScreen.Instance.CloseCategoryPanel(false);
            ToolMenu.Instance.ClearSelection();
            if (PlayerController.Instance.ActiveTool != SelectTool.Instance)
                PlayerController.Instance.ActivateTool(SelectTool.Instance);
            if (Selected)
                SelectTool.Instance.Select(null, true);
        }

        void Exit()
        {
            pop = Pop.None;
            CloseWork();
        }

        // ---- layout -------------------------------------------------------------------

        static void Show(CanvasGroup group, bool visible)
        {
            group.alpha = visible ? 1f : 0f;
            group.blocksRaycasts = visible;
            group.interactable = visible;
        }

        void Layout()
        {
            float width = Ui.Width;
            bool placing = Placing, category = Category, tool = ToolActive, selected = Selected;
            bool managementOpen = ManagementOpen, buildOpen = BuildOpen;

            // Top row, y 8 to 80.
            if (clock != null)
            {
                Ui.PlaceCorner(clock, 56f, 8f);
                clock.sizeDelta = new Vector2(82f, 82f);
                Ui.ScaleToHeight(clock, ClockSize);
            }
            if (speed != null)
            {
                Ui.PlaceCorner(speed, SpeedX, 16f);
                Ui.ScaleToHeight(speed, 52f);
            }
            float x = SpeedX + (speed != null ? speed.rect.width * speed.localScale.x / Ui.U : 240f) + 8f;
            if (redAlert != null)
            {
                Ui.PlaceCorner(redAlert, x, 16f);
                redAlert.sizeDelta = new Vector2(30f, 30f);
                Ui.ScaleToHeight(redAlert, 52f);
                x += 52f + 8f;
            }
            if (sandbox != null)
            {
                // The sandbox toggle as its icon only.
                Ui.PlaceCorner(sandbox, x, 16f);
                sandbox.sizeDelta = new Vector2(30f, 30f);
                Ui.ScaleToHeight(sandbox, 52f);
                Transform label = sandbox.Find("Label"), icon = sandbox.Find("FG");
                if (label != null && label.gameObject.activeSelf)
                    label.gameObject.SetActive(false);
                if (icon != null)
                {
                    var rect = (RectTransform)icon;
                    rect.anchorMin = rect.anchorMax = rect.pivot = new Vector2(0.5f, 0.5f);
                    rect.anchoredPosition = Vector2.zero;
                }
                x += 52f + 8f;
            }
            LayoutManagementBar(Margin);
            if (meters != null)
            {
                // Right of the speed controls, 40 high (the meters are 32 units high).
                Ui.PlaceCorner(meters, x + 8f, 22f);
                float scale = 40f * Ui.U / 32f;
                meters.localScale = new Vector3(scale, scale, 1f);
            }

            // Left column: diagnostics and resources, then notifications. Covered by B1 and
            // M1, so hidden with them.
            Show(left, !buildOpen && !managementOpen);
            Show(colonyGroup, !buildOpen && !managementOpen);
            float colonyHeight = LayoutUtility.GetPreferredHeight(colony) / Ui.U;
            Ui.Place(colony, 56f, 8f + ClockSize + 4f, 300f / ColonyScale, colonyHeight);
            colony.localScale = new Vector3(ColonyScale, ColonyScale, 1f);
            // The resources panel's "See All" link hangs under its own rectangle.
            Ui.Place(notifHolder, 56f, 8f + ClockSize + 4f + colonyHeight * ColonyScale + 28f, 0f, 0f);
            if (notifications != null)
            {
                notifications.anchorMin = notifications.anchorMax = new Vector2(0f, 1f);
                notifications.pivot = new Vector2(0f, 1f);
                notifications.anchoredPosition = Vector2.zero;
                notifications.localScale = new Vector3(0.8f, 0.8f, 1f);
            }
            // Its label hangs under the tile, as on the game's buttons.
            build.Visible = !buildOpen && !managementOpen;
            build.Place(56f, 620f, 64f, 64f);
            buildExit.Visible = buildOpen;
            buildExit.Place(56f, 620f, 64f, 64f);

            // B1: the categories in three columns, above the exit button; B2: the building
            // grid; B3: the chosen building.
            Show(b1, buildOpen && !placing);
            // A popover folds the building grid and the details away until it closes.
            Show(b2, category && !placing && pop == Pop.None);
            Show(b3, placing);
            Ui.Place(b1Holder, 56f, Top, 0f, 0f);
            var grid = planGrid.GetComponent<GridLayoutGroup>();
            if (grid != null)
            {
                grid.constraint = GridLayoutGroup.Constraint.FixedColumnCount;
                grid.constraintCount = 3;
                grid.startCorner = GridLayoutGroup.Corner.UpperLeft;
                grid.startAxis = GridLayoutGroup.Axis.Horizontal;
                // Room for the label under each icon.
                grid.spacing = new Vector2(4f, 22f);
            }
            foreach (Transform categoryButton in planGrid)
                ShortShadow(categoryButton);
            planGrid.anchorMin = planGrid.anchorMax = new Vector2(0f, 1f);
            planGrid.pivot = new Vector2(0f, 1f);
            planGrid.anchoredPosition = Vector2.zero;
            float cellScale = Cell * Ui.U / 45f;
            planGrid.localScale = new Vector3(cellScale, cellScale, 1f);

            Ui.Place(b2Holder, 56f + planGrid.rect.width * cellScale / Ui.U + 8f, Top, 0f, 0f);
            if (planPanel != null)
            {
                planPanel.anchorMin = planPanel.anchorMax = new Vector2(0f, 1f);
                planPanel.pivot = new Vector2(0f, 1f);
                planPanel.anchoredPosition = Vector2.zero;
                RectTransform groups = PlanScreen.Instance.buildingGroupsRoot;
                float scale = 320f * Ui.U / Mathf.Max(groups.sizeDelta.x, 1f);
                planPanel.localScale = new Vector3(scale, scale, 1f);
                groups.anchorMin = groups.anchorMax = new Vector2(0f, 1f);
                groups.pivot = new Vector2(0f, 1f);
                groups.anchoredPosition = Vector2.zero;
                groups.sizeDelta = new Vector2(groups.sizeDelta.x, 624f * Ui.U / scale);
            }
            Ui.Place(b3Holder, 56f, Top, 0f, 0f);
            if (productInfo != null)
            {
                // The building's description and effects start folded (its own header
                // unfolds them); what does not fit 624 is scaled down, to 70% at most.
                if (placing && !wasPlacing)
                    PlanScreen.Instance.ProductInfoScreen.ToggleExpandedInfo(false);
                productInfo.anchorMin = productInfo.anchorMax = new Vector2(0f, 1f);
                productInfo.pivot = new Vector2(0f, 1f);
                productInfo.anchoredPosition = Vector2.zero;
                float height = productInfo.rect.height;
                float scale = height > 1f ? Mathf.Clamp(624f * Ui.U / height, 0.7f, 1f) : 1f;
                productInfo.localScale = new Vector3(scale, scale, 1f);
            }
            wasPlacing = placing;
            rotate.Visible = placing && PlayerController.Instance.ActiveTool == BuildTool.Instance;
            rotate.Place(Margin + Cell + 24f, 622f, Cell, Cell, true);
            exit.Visible = tool;
            exit.Place(Margin, 622f, Cell, Cell, true);

            // H11: the tool rail, while no other work panel is open.
            Show(rail, !buildOpen && !selected && !managementOpen);
            Ui.Place(railHolder, Margin, Top, Cell, 7 * Step, true);
            foreach (RectTransform cell in railCells)
                FitCell(cell);
            more.Resize(Cell, Cell);
            more.Active = pop == Pop.MoreTools;

            toolName.gameObject.SetActive(tool && Time.unscaledTime < toolNameUntil);
            Ui.Place(toolName.rectTransform, (width - 400f) / 2f, 596f, 400f, 36f);
            toolName.fontSize = 22f * Ui.U;

            // P: the priority bar, bottom center.
            Ui.Place(pHolder, (width - 768f) / 2f, 640f, 768f, Cell);
            if (priority != null)
            {
                priority.anchorMin = priority.anchorMax = new Vector2(0.5f, 0.5f);
                priority.pivot = new Vector2(0.5f, 0.5f);
                priority.anchoredPosition = Vector2.zero;
                Ui.ScaleToHeight(priority, 64f);
            }
            if (toolParams != null)
            {
                // The tool filter, left of the rail, from the top. The game sizes it from
                // its own bottom right corner.
                Ui.Place(paramsHolder, Margin + Step + 8f, Top, 0f, 0f, true);
                toolParams.anchorMin = toolParams.anchorMax = new Vector2(1f, 1f);
                // Its list is a fixed height with the rows at the bottom.
                var list = toolParams.GetComponentInChildren<VerticalLayoutGroup>();
                float height = list != null ? LayoutUtility.GetPreferredHeight((RectTransform)list.transform) : 0f;
                toolParams.anchoredPosition = new Vector2(0f, -height);
            }

            // D1: details on the right, at most 624 high.
            Show(d1, pop == Pop.None);
            Ui.Place(d1Holder, Margin, Top, 0f, 0f, true);
            if (rootMenu != null)
            {
                rootMenu.anchorMin = rootMenu.anchorMax = new Vector2(1f, 1f);
                rootMenu.pivot = new Vector2(1f, 1f);
                rootMenu.anchoredPosition = Vector2.zero;
                rootMenu.localScale = Vector3.one;
                FitDetails();
            }

            if (sheetScroll != null)
                LayoutSheets();

            // O2: the overlay legend, bottom left, not while choosing a building.
            if (legend != null)
            {
                legend.anchorMin = legend.anchorMax = new Vector2(0f, 1f);
                legend.pivot = new Vector2(0f, 1f);
                legend.anchoredPosition = Vector2.zero;
                float height = legend.rect.height / Ui.U;
                float legendX = placing && productInfo != null ? 64f + productInfo.rect.width / Ui.U : 160f;
                Ui.PlaceCorner(legendHolder, legendX, Mathf.Max(Top, 712f - height));
            }
            legendHolder.gameObject.SetActive(!(buildOpen && !placing) && !managementOpen);

            // Slot B.
            if (managementOpen && pop != Pop.None)
                pop = Pop.None;
            scrim.SetActive(pop != Pop.None);
            Show(popover, pop != Pop.None);
            popMore.gameObject.SetActive(pop == Pop.MoreTools);
            PlacePopover(popMore, Margin + Step, 432f);
            // The overlay menu destroys and remakes its toggles in its own place when it
            // refreshes (input device, research).
            Adopt(overlaySource, overlayRow, true);
            foreach (Transform cell in overlayRow.Cast())
                FitCell(cell as RectTransform, OverlayCell);
            overlayRow.GetComponent<HorizontalLayoutGroup>().spacing = 2f * Ui.U;
            // Out of the way of the priority bar and the exit button while working.
            Show(overlayGroup, !tool && !placing && !managementOpen);
            Ui.Place(overlayRow, Margin, 712f - OverlayCell, Ui.Width - 2f * Margin, OverlayCell, true);
            SizeGrid(popMore, Cell, Cell);
            foreach (RectTransform cell in moreCells)
                FitCell(cell);

        }

        // The management bar, right aligned at right (reference pixels from the right
        // edge), its buttons 52 high and without their labels. The menu remakes its
        // buttons when it refreshes, so this runs every frame.
        void LayoutManagementBar(float right)
        {
            if (managementBar == null)
                return;
            foreach (Transform child in managementBar.Cast())
            {
                if (child.GetComponent<KToggle>() == null)
                    continue;
                bool keep = System.Array.IndexOf(DroppedManagement, child.name) < 0;
                if (child.gameObject.activeSelf != keep)
                    child.gameObject.SetActive(keep);
                Transform label = child.Find("TextContainer");
                if (label != null && label.gameObject.activeSelf)
                    label.gameObject.SetActive(false);
                var size = child.GetComponent<LayoutElement>();
                if (size != null)
                {
                    size.minWidth = size.preferredWidth = 40f;
                    size.flexibleWidth = 0f;
                }
            }
            // Level with the top right buttons (y 12 to 76); its layout pads the bottom.
            managementBar.anchorMin = managementBar.anchorMax = new Vector2(1f, 1f);
            managementBar.pivot = new Vector2(1f, 0.5f);
            managementBar.sizeDelta = new Vector2(LayoutUtility.GetPreferredWidth(managementBar),
                LayoutUtility.GetPreferredHeight(managementBar));
            float scale = 52f * Ui.U / 32f;
            managementBar.localScale = new Vector3(scale, scale, 1f);
            managementBar.anchoredPosition = new Vector2(-right * Ui.U, -(TopCell / 2f) * Ui.U);
        }

        // Moves the toggles a game menu (re)made under source into target, each in a
        // cell of its own, and drops the cells of destroyed toggles.
        static void Adopt(Transform source, RectTransform target, bool wrap)
        {
            if (source == null)
                return;
            foreach (Transform child in source.Cast())
            {
                if (child.GetComponent<KToggle>() == null)
                    continue;
                if (wrap)
                    Wrap((RectTransform)child, target, child.name);
                else
                    child.SetParent(target, false);
            }
            if (!wrap)
                return;
            foreach (Transform cell in target.Cast())
                if (cell.childCount == 0)
                    Object.Destroy(cell.gameObject);
        }

        static void SizeGrid(RectTransform panel, float w, float h)
        {
            var grid = panel.GetComponent<GridLayoutGroup>();
            grid.cellSize = new Vector2(w * Ui.U, h * Ui.U);
            grid.spacing = new Vector2(4f * Ui.U, 4f * Ui.U);
        }

        static void PlacePopover(RectTransform panel, float right, float y)
        {
            panel.anchorMin = panel.anchorMax = new Vector2(1f, 1f);
            panel.pivot = new Vector2(1f, 1f);
            panel.anchoredPosition = new Vector2(-right * Ui.U, -y * Ui.U);
        }

        // A cell takes the toggle's visibility (DLC and research hide some), and the
        // toggle fills it. The tool toggles' labels hang under the icon, over the next
        // cell, so they are hidden: tips mode reads them.
        static void FitCell(RectTransform cell, float size = Cell)
        {
            if (cell == null || cell.childCount == 0)
                return;
            var toggle = cell.GetChild(0) as RectTransform;
            if (toggle == null)
                return;
            if (cell.gameObject.activeSelf != toggle.gameObject.activeSelf)
                cell.gameObject.SetActive(toggle.gameObject.activeSelf);
            cell.sizeDelta = new Vector2(size * Ui.U, size * Ui.U);
            Ui.Stretch(toggle);
            toggle.localScale = Vector3.one;
            Transform label = toggle.Find("Text");
            if (label != null)
                label.localScale = Vector3.zero;
            ShortShadow(toggle);
        }

        // The game's drop shadow under a button, about a third as far (8, -5 units in the
        // game): at phone size the full one is heavy.
        static void ShortShadow(Transform button)
        {
            Transform shadow = button.Find("Shadow");
            if (shadow != null)
                ((RectTransform)shadow).anchoredPosition = new Vector2(2.8f, -1.8f);
        }

        // Shrinks the details body until the whole panel (title, tabs, body, buttons) is
        // at most 624 reference pixels high.
        void FitDetails()
        {
            if (detailsBody == null || DetailsScreen.Instance == null || !DetailsScreen.Instance.gameObject.activeInHierarchy)
                return;
            var details = DetailsScreen.Instance.transform as RectTransform;
            float excess = details.rect.height - 624f * Ui.U;
            if (Mathf.Abs(excess) < 2f)
                return;
            float body = detailsBody.preferredHeight > 0f ? detailsBody.preferredHeight : detailsBody.GetComponent<RectTransform>().rect.height;
            float target = Mathf.Max(120f * Ui.U, body - excess);
            if (excess < 0f)
                return;
            detailsBody.preferredHeight = target;
            detailsBody.minHeight = Mathf.Min(detailsBody.minHeight, target);
        }
    }

    static class TransformExtensions
    {
        // A copy of a transform's children, so they can be reparented while iterating.
        public static List<Transform> Cast(this Transform parent)
        {
            var children = new List<Transform>(parent.childCount);
            for (int i = 0; i < parent.childCount; i++)
                children.Add(parent.GetChild(i));
            return children;
        }
    }
}
