-- ServerRadio.lua - In-game live radio streaming addon for 3.3.5a
local addonName, addonTable = ...

-- Default database settings
ServerRadioDB = ServerRadioDB or {
    volume = 0.5,
    selectedStation = 1,
    customUrl = "",
    autoPlay = false,
    stopOnMinimize = true,
    showMinimap = true,
    minimapPos = 220,
}

local STATIONS = {
    { name = "Everlook Relay 1 (TWoW)", url = "http://radiodirect.turtle-music.org/stream" },
    { name = "Everlook Relay 2 (TWoW)", url = "http://radiodirect2.turtle-music.org/stream" },
    { name = "Nightwave Plaza (Vaporwave)", url = "http://radio.plaza.one/mp3" },
    { name = "SomaFM Groove Salad (Ambient)", url = "http://ice1.somafm.com/groovesalad-128-mp3" },
    { name = "Lofi Girl (Chill Beats)", url = "http://stream.zeno.fm/f3wvbbqmdg8uv" },
    { name = "SomaFM Secret Agent (Retro)", url = "http://ice1.somafm.com/secretagent-128-mp3" },
}

local function HasRadioSupport()
    return type(PlayRadioStream) == "function" and type(StopRadioStream) == "function"
end

-- -------------------------------------------------------------
-- Expanded Main Player Window (420 x 395)
-- -------------------------------------------------------------
local frame = CreateFrame("Frame", "ServerRadioFrame", UIParent)
frame:SetSize(420, 395)
frame:SetPoint("CENTER", UIParent, "CENTER", 0, 50)
frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
    tile = true, tileSize = 32, edgeSize = 24,
    insets = { left = 8, right = 8, top = 8, bottom = 8 }
})
frame:SetMovable(true)
frame:EnableMouse(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", frame.StartMoving)
frame:SetScript("OnDragStop", frame.StopMovingOrSizing)
frame:Hide()

-- Header Goblin Icon
local headerIcon = frame:CreateTexture(nil, "OVERLAY")
headerIcon:SetSize(28, 28)
headerIcon:SetPoint("TOPLEFT", frame, "TOPLEFT", 16, -14)
headerIcon:SetTexture("Interface\\Icons\\INV_Gizmo_GoblinBoomBox_01")
headerIcon:SetTexCoord(0.07, 0.93, 0.07, 0.93)

-- Header Title
local title = frame:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
title:SetPoint("LEFT", headerIcon, "RIGHT", 10, 4)
title:SetText("Everlook Broadcasting Co.")

local subtitle = frame:CreateFontString(nil, "OVERLAY", "GameFontDisableSmall")
subtitle:SetPoint("LEFT", headerIcon, "RIGHT", 10, -10)
subtitle:SetText("Live Azeroth Community Radio & Sound System")

-- Close Button
local closeBtn = CreateFrame("Button", nil, frame, "UIPanelCloseButton")
closeBtn:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -4, -4)

-- Separator Line 1
local sep1 = frame:CreateTexture(nil, "ARTWORK")
sep1:SetTexture("Interface\\TradeSkillFrame\\UI-TradeSkill-SkillDivider")
sep1:SetSize(388, 8)
sep1:SetPoint("TOP", frame, "TOP", 0, -48)

-- Section: Presets Header
local presetHeader = frame:CreateFontString(nil, "OVERLAY", "GameFontNormal")
presetHeader:SetPoint("TOPLEFT", frame, "TOPLEFT", 20, -58)
presetHeader:SetText("Broadcast Stations (Click to Tune In):")

-- Custom font for preset buttons (-1 font size to fit text cleanly)
local stationFont = CreateFont("ServerRadioStationFont")
local fontFile, fontSize, fontFlags = GameFontNormal:GetFont()
stationFont:SetFont(fontFile, fontSize - 1, fontFlags)
stationFont:SetTextColor(GameFontNormal:GetTextColor())

local stationHighlightFont = CreateFont("ServerRadioStationHighlightFont")
local hFontFile, hFontSize, hFontFlags = GameFontHighlight:GetFont()
stationHighlightFont:SetFont(hFontFile, hFontSize - 1, hFontFlags)
stationHighlightFont:SetTextColor(GameFontHighlight:GetTextColor())

-- Preset Station Buttons (2 columns of 3)
local stationButtons = {}
for i, station in ipairs(STATIONS) do
    local btn = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
    btn:SetSize(185, 24)
    if i % 2 == 1 then
        btn:SetPoint("TOPLEFT", frame, "TOPLEFT", 20, -78 - (math.floor((i - 1) / 2) * 28))
    else
        btn:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -20, -78 - (math.floor((i - 1) / 2) * 28))
    end
    btn:SetNormalFontObject(stationFont)
    btn:SetHighlightFontObject(stationHighlightFont)
    btn:SetText(station.name)
    btn:SetScript("OnClick", function()
        ServerRadioDB.selectedStation = i
        ServerRadio_PlayCurrent()
    end)
    stationButtons[i] = btn
end

-- Separator Line 2
local sep2 = frame:CreateTexture(nil, "ARTWORK")
sep2:SetTexture("Interface\\TradeSkillFrame\\UI-TradeSkill-SkillDivider")
sep2:SetSize(388, 8)
sep2:SetPoint("TOP", frame, "TOP", 0, -168)

-- Custom URL Header
local customHeader = frame:CreateFontString(nil, "OVERLAY", "GameFontNormal")
customHeader:SetPoint("TOPLEFT", frame, "TOPLEFT", 20, -180)
customHeader:SetText("Custom Stream URL (Icecast / Shoutcast / MP3 / OGG):")

-- Custom URL Input Box
local urlBg = CreateFrame("Frame", nil, frame)
urlBg:SetSize(380, 24)
urlBg:SetPoint("TOP", frame, "TOP", 0, -200)
urlBg:SetBackdrop({
    bgFile = "Interface\\ChatFrame\\ChatFrameBackground",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    tile = true, tileSize = 16, edgeSize = 12,
    insets = { left = 3, right = 3, top = 3, bottom = 3 }
})
urlBg:SetBackdropColor(0, 0, 0, 0.6)
urlBg:SetBackdropBorderColor(0.6, 0.6, 0.6, 0.8)

local urlEdit = CreateFrame("EditBox", "ServerRadioUrlEdit", urlBg)
urlEdit:SetSize(370, 18)
urlEdit:SetPoint("CENTER", urlBg, "CENTER", 0, 0)
urlEdit:SetFontObject("GameFontHighlight")
urlEdit:SetAutoFocus(false)
urlEdit:SetText(ServerRadioDB.customUrl or "http://")
urlEdit:SetScript("OnEnterPressed", function(self)
    self:ClearFocus()
    ServerRadioDB.customUrl = self:GetText()
end)

-- Now Playing / Status Box
local statusBg = CreateFrame("Frame", nil, frame)
statusBg:SetSize(380, 42)
statusBg:SetPoint("TOP", urlBg, "BOTTOM", 0, -10)
statusBg:SetBackdrop({
    bgFile = "Interface\\ChatFrame\\ChatFrameBackground",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    tile = true, tileSize = 16, edgeSize = 12,
    insets = { left = 3, right = 3, top = 3, bottom = 3 }
})
statusBg:SetBackdropColor(0.05, 0.05, 0.1, 0.7)
statusBg:SetBackdropBorderColor(0.8, 0.7, 0.2, 0.8)

local statusText = statusBg:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
statusText:SetPoint("CENTER", statusBg, "CENTER", 0, 0)
statusText:SetWidth(360)
statusText:SetHeight(32)
statusText:SetText("Status: |cFF888888Stopped|r")

-- Controls: Play, Stop Buttons
local playBtn = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
playBtn:SetSize(90, 26)
playBtn:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 20, 44)
playBtn:SetNormalFontObject(stationFont)
playBtn:SetHighlightFontObject(stationHighlightFont)
playBtn:SetText("Play")
playBtn:SetScript("OnClick", function()
    ServerRadio_PlayCurrent()
end)

local stopBtn = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
stopBtn:SetSize(90, 26)
stopBtn:SetPoint("LEFT", playBtn, "RIGHT", 10, 0)
stopBtn:SetNormalFontObject(stationFont)
stopBtn:SetHighlightFontObject(stationHighlightFont)
stopBtn:SetText("Stop")
stopBtn:SetScript("OnClick", function()
    ServerRadio_Stop()
end)

-- Volume Slider
local volSlider = CreateFrame("Slider", "ServerRadioVolSlider", frame, "OptionsSliderTemplate")
volSlider:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -25, 46)
volSlider:SetMinMaxValues(0, 100)
volSlider:SetValue((ServerRadioDB.volume or 0.5) * 100)
volSlider:SetValueStep(5)
volSlider:SetWidth(160)
_G[volSlider:GetName() .. "Low"]:SetText("0%")
_G[volSlider:GetName() .. "High"]:SetText("100%")
_G[volSlider:GetName() .. "Text"]:SetText("Volume: " .. math.floor((ServerRadioDB.volume or 0.5) * 100) .. "%")

local function SafeCall(fn, ...)
    if type(fn) == "function" then
        local ok, res = pcall(fn, ...)
        if ok then return res end
    end
    return nil
end

volSlider:SetScript("OnValueChanged", function(self, value)
    local vol = value / 100
    ServerRadioDB.volume = vol
    _G[self:GetName() .. "Text"]:SetText("Volume: " .. math.floor(value) .. "%")
    if HasRadioSupport() and SetRadioVolume then
        SafeCall(SetRadioVolume, vol)
    end
end)

-- Synchronization helper
local function SyncAutoPauseState(val)
    ServerRadioDB.stopOnMinimize = val
    if stopOnMinCheck then stopOnMinCheck:SetChecked(val) end
    if ServerRadioOptAutoPause then ServerRadioOptAutoPause:SetChecked(val) end
    if HasRadioSupport() then
        if SetRadioAutoPause then
            SafeCall(SetRadioAutoPause, val and 1 or 0)
        elseif SetRadioStopOnMinimize then
            SafeCall(SetRadioStopOnMinimize, val and 1 or 0)
        end
    end
end

-- Auto-Pause Checkbox on Main Window (covers minimize & windowed background)
local stopOnMinCheck = CreateFrame("CheckButton", "ServerRadioStopOnMinCheck", frame, "UICheckButtonTemplate")
stopOnMinCheck:SetSize(22, 22)
stopOnMinCheck:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 18, 12)
_G[stopOnMinCheck:GetName() .. "Text"]:SetText("Auto-pause when minimized or leaving window (Auto-resumes on focus)")
_G[stopOnMinCheck:GetName() .. "Text"]:SetFontObject("GameFontNormalSmall")
stopOnMinCheck:SetChecked(true)
stopOnMinCheck:SetScript("OnClick", function(self)
    local isChecked = self:GetChecked() and true or false
    SyncAutoPauseState(isChecked)
end)

-- -------------------------------------------------------------
-- Blizzard Interface Options Panel (ESC -> Interface -> AddOns -> Server Radio)
-- -------------------------------------------------------------
local optionsPanel = CreateFrame("Frame", "ServerRadioOptionsPanel", UIParent)
optionsPanel.name = "Server Radio"

local optTitle = optionsPanel:CreateFontString(nil, "ARTWORK", "GameFontNormalLarge")
optTitle:SetPoint("TOPLEFT", 16, -16)
optTitle:SetText("Server Radio Settings")

local optSubtext = optionsPanel:CreateFontString(nil, "ARTWORK", "GameFontHighlightSmall")
optSubtext:SetPoint("TOPLEFT", optTitle, "BOTTOMLEFT", 0, -8)
optSubtext:SetText("Live streaming radio for WoW 3.3.5a (Everlook Broadcasting Co.).")

local optAutoPause = CreateFrame("CheckButton", "ServerRadioOptAutoPause", optionsPanel, "UICheckButtonTemplate")
optAutoPause:SetPoint("TOPLEFT", optSubtext, "BOTTOMLEFT", -2, -16)
_G[optAutoPause:GetName() .. "Text"]:SetText("Auto-pause when game is minimized or leaving window (auto-resumes when focused)")
_G[optAutoPause:GetName() .. "Text"]:SetFontObject("GameFontNormal")
optAutoPause:SetChecked(true)
optAutoPause:SetScript("OnClick", function(self)
    local isChecked = self:GetChecked() and true or false
    SyncAutoPauseState(isChecked)
end)

local optMinimap = CreateFrame("CheckButton", "ServerRadioOptMinimap", optionsPanel, "UICheckButtonTemplate")
optMinimap:SetPoint("TOPLEFT", optAutoPause, "BOTTOMLEFT", 0, -8)
_G[optMinimap:GetName() .. "Text"]:SetText("Show Minimap Icon (drag around map to position)")
_G[optMinimap:GetName() .. "Text"]:SetFontObject("GameFontNormal")
optMinimap:SetChecked(true)
optMinimap:SetScript("OnClick", function(self)
    local isChecked = self:GetChecked() and true or false
    ServerRadioDB.showMinimap = isChecked
    if ServerRadio_ToggleButton then
        if isChecked then ServerRadio_ToggleButton:Show() else ServerRadio_ToggleButton:Hide() end
    end
end)

local optOpenBtn = CreateFrame("Button", nil, optionsPanel, "UIPanelButtonTemplate")
optOpenBtn:SetSize(160, 26)
optOpenBtn:SetPoint("TOPLEFT", optMinimap, "BOTTOMLEFT", 2, -16)
optOpenBtn:SetText("Open Radio Player")
optOpenBtn:SetScript("OnClick", function()
    if frame:IsShown() then
        frame:Hide()
    else
        frame:Show()
    end
end)

optionsPanel.refresh = function()
    optAutoPause:SetChecked(ServerRadioDB.stopOnMinimize ~= false)
    optMinimap:SetChecked(ServerRadioDB.showMinimap ~= false)
end
optionsPanel.default = function()
    SyncAutoPauseState(true)
    ServerRadioDB.showMinimap = true
    optMinimap:SetChecked(true)
    if ServerRadio_ToggleButton then ServerRadio_ToggleButton:Show() end
end

InterfaceOptions_AddCategory(optionsPanel)

-- -------------------------------------------------------------
-- Play/Stop Logic
-- -------------------------------------------------------------
function ServerRadio_PlayCurrent()
    if not HasRadioSupport() then
        statusText:SetText("|cFFFF0000dinput8.dll not active! Restart WoW.|r")
        return
    end

    local url = ""
    local custom = urlEdit:GetText()
    if custom and custom:match("^https?://.+") and custom ~= "http://" then
        url = custom
        statusText:SetText("|cFF00FF00Connecting to Custom Stream...|r")
    else
        local st = STATIONS[ServerRadioDB.selectedStation or 1]
        url = st.url
        statusText:SetText("|cFF00FF00Tuned to: " .. st.name .. "|r")
    end

    if SetRadioVolume then
        SafeCall(SetRadioVolume, ServerRadioDB.volume or 0.5)
    end
    SafeCall(PlayRadioStream, url)
end

function ServerRadio_Stop()
    if HasRadioSupport() and StopRadioStream then
        SafeCall(StopRadioStream)
    end
    statusText:SetText("Status: |cFF888888Stopped|r")
end

-- Periodic ticker to update song title
local updateTimer = 0
frame:SetScript("OnUpdate", function(self, elapsed)
    updateTimer = updateTimer + elapsed
    if updateTimer >= 2.0 then
        updateTimer = 0
        if HasRadioSupport() and IsRadioPlaying and SafeCall(IsRadioPlaying) then
            local title = GetRadioTitle and SafeCall(GetRadioTitle)
            if title and title ~= "" then
                statusText:SetText("|cFF00FF00LIVE: |r" .. title)
            end
        end
    end
end)

-- -------------------------------------------------------------
-- Minimap Toggle Button (QOLAddon-style circular launcher positioned around Minimap)
-- -------------------------------------------------------------
local minimapBtn

local function UpdateMinimapButtonPos()
    if not minimapBtn then return end
    local angle = math.rad(ServerRadioDB.minimapPos or 220)
    local cos = math.cos(angle)
    local sin = math.sin(angle)
    -- Positioned around the Minimap circumference (radius 80)
    local x = cos * 80
    local y = sin * 80
    minimapBtn:ClearAllPoints()
    minimapBtn:SetPoint("CENTER", Minimap, "CENTER", x, y)
end

function ServerRadio_CreateToggleButton()
    if minimapBtn then return minimapBtn end

    local SIZE = 32
    local b = CreateFrame("Button", "ServerRadio_ToggleButton", Minimap)
    b:SetSize(SIZE, SIZE)
    b:SetFrameStrata("MEDIUM")
    b:SetFrameLevel(8)
    b:SetMovable(true)
    b:SetClampedToScreen(true)
    b:EnableMouse(true)
    b:RegisterForClicks("LeftButtonUp", "RightButtonUp")
    b:RegisterForDrag("LeftButton")

    -- Inner Icon (Goblin Radio Icon matching main header)
    local icon = b:CreateTexture(nil, "BACKGROUND")
    icon:SetTexture("Interface\\Icons\\INV_Gizmo_GoblinBoomBox_01")
    icon:SetSize(20, 20)
    icon:SetPoint("CENTER", b, "CENTER", 0, 0)
    icon:SetTexCoord(0.08, 0.92, 0.08, 0.92)
    if icon.SetMask then
        icon:SetMask("Interface\\CharacterFrame\\TempPortraitAlphaMask")
    end
    b.icon = icon

    -- Border (Standard Tracking Ring Border)
    local border = b:CreateTexture(nil, "OVERLAY")
    border:SetTexture("Interface\\Minimap\\MiniMap-TrackingBorder")
    border:SetSize(54, 54)
    border:SetPoint("CENTER", b, "CENTER", 11, -11)
    b.border = border

    -- Highlight Texture
    b:SetHighlightTexture("Interface\\Minimap\\UI-Minimap-ZoomButton-Highlight", "ADD")

    -- Dragging smoothly around the Minimap circumference
    b:SetScript("OnDragStart", function(self)
        self.isMoving = true
        self:SetScript("OnUpdate", function(self)
            local mx, my = Minimap:GetCenter()
            local cx, cy = GetCursorPosition()
            local scale = UIParent:GetEffectiveScale()
            cx, cy = cx / scale, cy / scale
            local angle = math.deg(math.atan2(cy - my, cx - mx))
            if angle < 0 then angle = angle + 360 end
            ServerRadioDB.minimapPos = angle
            UpdateMinimapButtonPos()
        end)
    end)

    b:SetScript("OnDragStop", function(self)
        self.isMoving = false
        self:SetScript("OnUpdate", nil)
    end)

    -- Clicks: Left = Toggle UI, Right = Toggle Play/Stop
    b:SetScript("OnClick", function(self, btn)
        if btn == "RightButton" then
            if HasRadioSupport() and IsRadioPlaying and SafeCall(IsRadioPlaying) then
                ServerRadio_Stop()
            else
                ServerRadio_PlayCurrent()
            end
        else
            if frame:IsShown() then
                frame:Hide()
            else
                frame:Show()
                frame:Raise()
            end
        end
    end)

    -- Tooltip on Hover (matches QOLAddon style & styling)
    b:SetScript("OnEnter", function(self)
        GameTooltip:SetOwner(self, "ANCHOR_LEFT")
        GameTooltip:SetText("Server Radio", 1, 0.79, 0.30)
        GameTooltip:AddLine("Everlook Broadcasting Co.", 1, 1, 1)

        local currentStation = STATIONS[ServerRadioDB.selectedStation or 1]
        local custom = urlEdit and urlEdit:GetText() or ServerRadioDB.customUrl
        local stationName = (custom and custom:match("^https?://.+") and custom ~= "http://") and "Custom Stream" or (currentStation and currentStation.name or "Unknown")
        GameTooltip:AddLine("Station: |cFFFFFFFF" .. stationName .. "|r", 0.8, 0.8, 0.8)

        local isPlaying = HasRadioSupport() and IsRadioPlaying and SafeCall(IsRadioPlaying)
        if isPlaying then
            local title = GetRadioTitle and SafeCall(GetRadioTitle)
            if title and title ~= "" then
                GameTooltip:AddLine("Playing: |cFF00FF00" .. title .. "|r", 0.8, 0.8, 0.8)
            else
                GameTooltip:AddLine("Status: |cFF00FF00Streaming|r", 0.8, 0.8, 0.8)
            end
        else
            GameTooltip:AddLine("Status: |cFF888888Stopped|r", 0.8, 0.8, 0.8)
        end

        GameTooltip:AddLine(" ", 1, 1, 1)
        GameTooltip:AddLine("|cFF00CCFFLeft-Click:|r Toggle Radio Player", 0.7, 0.7, 0.7)
        GameTooltip:AddLine("|cFF00CCFFRight-Click:|r Play / Stop Audio", 0.7, 0.7, 0.7)
        GameTooltip:AddLine("|cFF00CCFFDrag:|r Move around Minimap", 0.7, 0.7, 0.7)
        GameTooltip:AddLine("/radio minimap to toggle icon", 0.5, 0.5, 0.5)
        GameTooltip:Show()
    end)

    b:SetScript("OnLeave", function()
        GameTooltip:Hide()
    end)

    minimapBtn = b
    _G["ServerRadioMinimapButton"] = b

    UpdateMinimapButtonPos()

    if ServerRadioDB.showMinimap == false then
        b:Hide()
    else
        b:Show()
    end

    return b
end

-- Slash Command Handler
SLASH_SERVERRADIO1 = "/radio"
SLASH_SERVERRADIO2 = "/stream"
SlashCmdList["SERVERRADIO"] = function(msg)
    msg = (msg or ""):trim():lower()
    if msg == "play" then
        ServerRadio_PlayCurrent()
    elseif msg == "stop" then
        ServerRadio_Stop()
    elseif msg == "min" or msg == "pause" or msg == "bg" then
        local newVal = not (ServerRadioDB.stopOnMinimize ~= false)
        SyncAutoPauseState(newVal)
        DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Auto-pause on minimize/background: " .. (newVal and "|cFF00FF00Enabled|r" or "|cFFFF0000Disabled|r"))
    elseif msg == "minimap" or msg == "icon" or msg == "btn" or msg == "map" then
        ServerRadioDB.showMinimap = not (ServerRadioDB.showMinimap ~= false)
        if minimapBtn then
            if ServerRadioDB.showMinimap then minimapBtn:Show() else minimapBtn:Hide() end
        end
        if ServerRadioOptMinimap then
            ServerRadioOptMinimap:SetChecked(ServerRadioDB.showMinimap)
        end
        DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Minimap button: " .. (ServerRadioDB.showMinimap and "|cFF00FF00Shown|r" or "|cFFFF0000Hidden|r"))
    elseif msg == "reset" then
        ServerRadioDB.minimapPos = 220
        ServerRadioDB.showMinimap = true
        UpdateMinimapButtonPos()
        if minimapBtn then minimapBtn:Show() end
        if ServerRadioOptMinimap then ServerRadioOptMinimap:SetChecked(true) end
        frame:ClearAllPoints()
        frame:SetPoint("CENTER", UIParent, "CENTER", 0, 50)
        DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Player frame and minimap icon positions reset.")
    elseif msg:match("^vol%s*(%d+)$") then
        local volVal = tonumber(msg:match("^vol%s*(%d+)$"))
        if volVal then
            volSlider:SetValue(volVal)
        end
    else
        if frame:IsShown() then
            frame:Hide()
        else
            frame:Show()
        end
    end
end

-- Initialize on addon loaded / enter world
local loaderFrame = CreateFrame("Frame")
loaderFrame:RegisterEvent("ADDON_LOADED")
loaderFrame:RegisterEvent("PLAYER_ENTERING_WORLD")
loaderFrame:SetScript("OnEvent", function(self, event, arg1)
    if event == "ADDON_LOADED" and arg1 == "ServerRadio" then
        if ServerRadioDB.stopOnMinimize == nil then
            ServerRadioDB.stopOnMinimize = true
        end
        if ServerRadioDB.showMinimap == nil then
            ServerRadioDB.showMinimap = true
        end
        if ServerRadioDB.minimapPos == nil then
            ServerRadioDB.minimapPos = 220
        end
        SyncAutoPauseState(ServerRadioDB.stopOnMinimize ~= false)
        ServerRadio_CreateToggleButton()
    elseif event == "PLAYER_ENTERING_WORLD" then
        if ServerRadioDB.stopOnMinimize == nil then
            ServerRadioDB.stopOnMinimize = true
        end
        if ServerRadioDB.showMinimap == nil then
            ServerRadioDB.showMinimap = true
        end
        if ServerRadioDB.minimapPos == nil then
            ServerRadioDB.minimapPos = 220
        end
        SyncAutoPauseState(ServerRadioDB.stopOnMinimize ~= false)
        ServerRadio_CreateToggleButton()

        if HasRadioSupport() then
            DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Everlook Broadcasting Co. loaded! Type |cFFFFFF00/radio|r or click the Minimap icon.")
        else
            DEFAULT_CHAT_FRAME:AddMessage("|cFFFF8800[ServerRadio]|r Addon loaded. Type |cFFFFFF00/radio|r or click the Minimap icon.")
        end
    end
end)
