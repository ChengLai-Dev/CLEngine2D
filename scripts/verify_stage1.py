"""阶段 1 验证脚本：E1 动态建 UI + E2 Label 折行 + 多 .cui 叠加层。

运行方式（工作目录 = 项目根）：
    out/build/default/src/py_sandbox/Debug/py_sandbox.exe --module verify_stage1

验证点：
  1. 动态创建 Button 挂载到 Layout -> DoLayout 生效 -> 点击回调触发（人工点击）
  2. 动态创建 Label 挂载到已有节点，显示正常（含生僻字）
  3. 580px 宽 / 字号 28 长文本自动折行 + '\\n' 分段 + 行距/左对齐（目测）
  4. push/pop 场景：动态节点随场景切换正常销毁（无崩溃）
  5. 全部 10 个 .cui 可加载（UISystem.AddUI 层挂载）
  6. 多 .cui 叠加：UISystem.AddUI 叠加层 / RemoveUI 摘除 / zOrder / 整树显隐
"""

from CLEngine.SceneGraph import (
    Scene, SceneManager, UISystem,
    CreateButton, CreateLabel, CreateSprite,
    LoadTexture,
    TextAlign, WrapText, MeasureText,
)
from CLEngine.Math import Vec2, Vec3
from CLEngine.Renderer import SetClearColor
import CLEngine

scene = None
dynamic_btn = None
wrap_label = None
click_count = 0


def log(msg):
    print(msg, flush=True)


def on_init():
    global scene, dynamic_btn, wrap_label, click_count

    SetClearColor(0.1, 0.1, 0.2, 1.0)
    scene = Scene()
    SceneManager.GetInstance().PushScene(scene)

    ui_root = UISystem.GetInstance().AddUI("assets/ui/DialogScene.cui", 0)
    log(f"[verify] 1.1 DialogScene 挂载为层: {'OK' if ui_root is not None else 'FAIL'}")
    if not ui_root:
        log("[verify] FAIL: UISystem.AddUI 返回空")
        return

    # --- 验证点 2：动态 Label 挂载到已有节点（DialogBox）---
    dialog_box = ui_root.FindChild("DialogBox")
    dyn_label = CreateLabel(dialog_box, "DynLabel")
    dyn_label.SetPosition(Vec3(-200, -30, 0))
    dyn_label.SetContentSize(Vec2(400, 40))
    dyn_label.SetText("动态 Label：蚀之影 · 烬火")
    dyn_label.SetFontSize(24)
    found = ui_root.FindChild("DynLabel")
    log(f"[verify] 2.1 动态 Label 创建并挂载(FindChild): {'OK' if found is not None else 'FAIL'}")

    # --- 验证点 1：动态 Button 挂载到 OptionLayout（Layout 集成）---
    option_layout = ui_root.FindChild("OptionLayout")
    if not option_layout:
        log("[verify] FAIL: OptionLayout 未找到")
        return

    dynamic_btn = CreateButton(option_layout, "DynOption0")
    dynamic_btn.SetContentSize(Vec2(480, 64))
    dynamic_btn.SetText("动态选项按钮")
    dynamic_btn.SetFontSize(24)
    normal_tex = LoadTexture("assets/placeholder/ui/btn_main_normal.png")
    pressed_tex = LoadTexture("assets/placeholder/ui/btn_main_pressed.png")
    dynamic_btn.SetNormalImage(normal_tex)
    dynamic_btn.SetPressedImage(pressed_tex)
    option_layout.DoLayout()
    pos = dynamic_btn.GetPosition()
    log(f"[verify] 1.1 动态 Button 挂入 Layout 并 DoLayout，位置=({pos.x:.1f}, {pos.y:.1f})")

    def on_clicked(btn):
        global click_count
        click_count += 1
        log(f"[verify] 1.2 动态 Button 点击回调触发 #{click_count}")

    dynamic_btn.OnClicked(on_clicked)
    log("[verify] 1.3 请在窗口内点击\"动态选项按钮\"，回调触发会打印日志")

    # --- 验证点 3：折行 Label（580x140，字号 28，左对齐 + 行距 1.6）---
    wrap_label = CreateLabel(ui_root, "WrapTest")
    wrap_label.SetPosition(Vec3(0, 220, 0))
    wrap_label.SetContentSize(Vec2(580, 140))
    wrap_label.SetFontSize(28)
    wrap_label.SetHAlign(TextAlign.LEFT)
    wrap_label.SetLineSpacing(1.6)
    wrap_label.SetText(
        "——守夜人，星环的裂隙又扩大了。蚀之影正从渊底蔓延，烬火即将熄灭。\n"
        "若你执意前往，请记住：光与水，永远相生。"
    )
    log("[verify] 3.1 折行 Label 已创建：580px 宽 / 字号 28 / 左对齐 / 行距 1.6")
    log("[verify] 3.2 请目测：长文本自动折行完整显示、\\n 分段正确、生僻字(蚀/烬)正常")

    # 折行逻辑自动断言（580px 宽 / 字号 28 -> scale = 2.0）
    test_text = (
        "——守夜人，星环的裂隙又扩大了。蚀之影正从渊底蔓延，烬火即将熄灭。\n"
        "若你执意前往，请记住：光与水，永远相生。"
    )
    lines = WrapText(test_text, 580.0, 2.0)
    widths = [MeasureText(line, 2.0).x for line in lines]
    in_bounds = all(w <= 580.0 + 0.5 for w in widths)
    forced_break_ok = len(lines) == 3
    log(f"[verify] 3.3 WrapText(580, scale=2.0) -> {len(lines)} 行，各行宽度 {[round(w, 1) for w in widths]}")
    log(f"[verify] 3.4 每行不超宽: {'OK' if in_bounds else 'FAIL'}；\\n 分段(3行): {'OK' if forced_break_ok else 'FAIL'}")

    # --- 验证点 4：动态节点随场景切换正常销毁 ---
    tmp_scene = Scene()
    tmp_root = tmp_scene.GetRoot()
    tmp_sprite = CreateSprite(tmp_root, "TmpSprite")
    tmp_sprite.SetContentSize(Vec2(32, 32))
    tmp_sprite.SetPosition(Vec3(300, 300, 0))
    SceneManager.GetInstance().PushScene(tmp_scene)
    SceneManager.GetInstance().PopScene()
    log("[verify] 4.1 push/pop 临时场景完成：动态节点已随场景销毁（无崩溃）")

    # --- 回归检查：全部 10 个 .cui 可加载（UISystem.AddUI 层挂载）---
    cui_files = [
        "Title", "DialogScene", "BattleScene", "BattleResult", "GameOver",
        "ChapterTransition", "ExploreScene", "Settings", "Ending", "SaveLoad",
    ]
    for name in cui_files:
        layer = UISystem.GetInstance().AddUI(f"assets/ui/{name}.cui", 5)
        ok = layer is not None
        UISystem.GetInstance().RemoveUI(layer) if layer else None
        log(f"[verify] 5.1 {name}.cui 挂载/摘除: {'OK' if ok else 'FAIL'}")
    log("[verify] 5.2 全部 .cui 挂载-摘除完成（主层动态元素保留）")

    # --- 验证点 6：多 .cui 叠加（UISystem.AddUI / RemoveUI）---
    layer_count_before = len(UISystem.GetInstance().GetLayers())

    overlay = UISystem.GetInstance().AddUI("assets/ui/Settings.cui", 10)
    log(f"[verify] 6.1 AddUI(Settings.cui, zorder=10): {'OK' if overlay is not None else 'FAIL'}，"
        f"层数 {layer_count_before} -> {len(UISystem.GetInstance().GetLayers())}")

    back_btn = overlay.FindChild("BtnBack") if overlay else None
    log(f"[verify] 6.2 叠加层 FindChild(BtnBack): {'OK' if back_btn is not None else 'FAIL'}")

    overlay.SetVisible(False)
    hidden_ok = not overlay.IsVisible()
    overlay.SetVisible(True)
    log(f"[verify] 6.3 叠加层整树显隐(SetVisible): {'OK' if hidden_ok else 'FAIL'}")

    removed = UISystem.GetInstance().RemoveUI(overlay)
    restored_ok = len(UISystem.GetInstance().GetLayers()) == layer_count_before
    log(f"[verify] 6.4 RemoveUI(overlay): {'OK' if removed else 'FAIL'}，"
        f"层数 {len(UISystem.GetInstance().GetLayers())} -> {layer_count_before} 恢复: {'OK' if restored_ok else 'FAIL'}")

    log("[verify] 6.5 叠加验证完成（AddUI/FindChild/显隐/RemoveUI 全自动断言）")

    log("[verify] 全部就绪，等待点击验证...")


def on_update(dt):
    if scene:
        scene.OnUpdate(dt)
        UISystem.GetInstance().ProcessEvents()


def on_render():
    pass


def on_shutdown():
    log(f"[verify] 结束：动态按钮点击回调共触发 {click_count} 次")
