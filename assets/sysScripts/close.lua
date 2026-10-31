--[[
close([name])
- 無參數：預設關閉 UIConsole ("InGameConsole")
- 有參數：依名稱尋找該 UI 物件並設為不可見
--]]
function close(name)
local targetName = name or "InGameConsole"
local elem = FindUI(targetName)

if elem then
    UI_SetVisible(elem, false)
    EngineLog("[System] 已關閉 UI 元件: " .. targetName)
    return true
    else
        EngineLog("[System] 找不到指定的 UI 元件: " .. targetName)
        return false
    end
end
