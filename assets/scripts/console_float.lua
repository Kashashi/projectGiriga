-- assets/scripts/console_float.lua

EngineLog("載入主控台動態腳本...")

local console = FindUI("GlobalConsole")

if console then
    local uuid = UI_GetUUID(console)
    EngineLog("成功取得全域主控台實例！UUID: " .. tostring(uuid))

    local total_time = 0.0
    -- 初始位置取物件目前的座標
    local base_x = UI_GetX(console)
    local base_y = UI_GetY(console)

    BindUIUpdate(console, function(self, dt)
    -- 如果玩家正在拖曳視窗，動畫暫停，並記錄玩家拖去哪裡
    if UI_IsDragging(self) then
        base_x = UI_GetX(self)
        base_y = UI_GetY(self)
        -- 重設動畫相位，避免放開滑鼠瞬間座標跳變
        total_time = 0.0
        return
        end

        total_time = total_time + dt

        -- 正弦波上下呼吸浮動 (振幅 10px，頻率 2.5)
    local offset_y = math.sin(total_time * 2.5) * 10.0

    UI_SetPosition(self, base_x, base_y + offset_y)
    end)
    else
        EngineLog("[錯誤] 找不到名稱為 GlobalConsole 的 UI 元件！")
        end
