-- assets/scripts/console_behavior.lua

local console = FindUI("GlobalConsole")

if console then
    EngineLog("成功找到全域主控台，UUID 為: " .. tostring(UI_GetUUID(console)))

    local total_time = 0.0
    BindUIUpdate(console, function(self, dt)
    total_time = total_time + dt
    -- 動態飄浮動畫
    local offset_y = math.sin(total_time * 2.0) * 10.0
    UI_SetPosition(self, 200.0, 150.0 + offset_y)
    end)
    else
        EngineLog("找不到指定的 UI 元件！")
        end
