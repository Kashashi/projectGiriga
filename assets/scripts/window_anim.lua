-- 為 C++ 端傳入的視窗物件綁定幀更新
function setup_window_behavior(window)
    local total_time = 0.0
    local origin_y = 150.0

    BindUIUpdate(window, function(self, dt)
        total_time = total_time + dt

        -- 讓視窗隨時間做弦波上下浮動 (Float Animation)
        local offset_y = math.sin(total_time * 2.0) * 15.0

        -- 調用 C++ 暴露的 setPosition API
        UI_SetPosition(self, 200.0, origin_y + offset_y)
    end)
end
