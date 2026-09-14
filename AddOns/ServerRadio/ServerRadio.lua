-- ServerRadio.lua - In-game live radio streaming addon for 3.3.5a
local addonName, addonTable = ...

-- Default database settings
ServerRadioDB = ServerRadioDB or {
    volume = 0.5,
    selectedStation = 1,
    customUrl = "",
    autoPlay = false,
    stopOnMinimize = true,
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
headerIcon:SetTexture("Interface\\Icons\\INV_Misc_Head_Goblin_01")
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
playBtn:SetText("Play")
playBtn:SetScript("OnClick", function()
    ServerRadio_PlayCurrent()
end)

local stopBtn = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
stopBtn:SetSize(90, 26)
stopBtn:SetPoint("LEFT", playBtn, "RIGHT", 10, 0)
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

volSlider:SetScript("OnValueChanged", function(self, value)
    local vol = value / 100
    ServerRadioDB.volume = vol
    _G[self:GetName() .. "Text"]:SetText("Volume: " .. math.floor(value) .. "%")
    if HasRadioSupport() and SetRadioVolume then
        SetRadioVolume(vol)
    end
end)

-- Minimize Auto-Pause Checkbox
local stopOnMinCheck = CreateFrame("CheckButton", "ServerRadioStopOnMinCheck", frame, "UICheckButtonTemplate")
stopOnMinCheck:SetSize(22, 22)
stopOnMinCheck:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 18, 12)
_G[stopOnMinCheck:GetName() .. "Text"]:SetText("Auto-pause when game is minimized (Auto-resumes on restore)")
_G[stopOnMinCheck:GetName() .. "Text"]:SetFontObject("GameFontNormalSmall")
stopOnMinCheck:SetChecked(ServerRadioDB.stopOnMinimize ~= false)
stopOnMinCheck:SetScript("OnClick", function(self)
    local isChecked = self:GetChecked() and true or false
    ServerRadioDB.stopOnMinimize = isChecked
    if SetRadioStopOnMinimize then
        SetRadioStopOnMinimize(isChecked and 1 or 0)
    end
end)

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
        SetRadioVolume(ServerRadioDB.volume or 0.5)
    end
    PlayRadioStream(url)
end

function ServerRadio_Stop()
    if HasRadioSupport() and StopRadioStream then
        StopRadioStream()
    end
    statusText:SetText("Status: |cFF888888Stopped|r")
end

-- Periodic ticker to update song title
local updateTimer = 0
frame:SetScript("OnUpdate", function(self, elapsed)
    updateTimer = updateTimer + elapsed
    if updateTimer >= 2.0 then
        updateTimer = 0
        if HasRadioSupport() and IsRadioPlaying and IsRadioPlaying() then
            local title = GetRadioTitle and GetRadioTitle()
            if title and title ~= "" then
                statusText:SetText("|cFF00FF00LIVE: |r" .. title)
            end
        end
    end
end)

-- Slash Command Handler
SLASH_SERVERRADIO1 = "/radio"
SLASH_SERVERRADIO2 = "/stream"
SlashCmdList["SERVERRADIO"] = function(msg)
    msg = (msg or ""):trim():lower()
    if msg == "play" then
        ServerRadio_PlayCurrent()
    elseif msg == "stop" then
        ServerRadio_Stop()
    elseif msg == "min" then
        ServerRadioDB.stopOnMinimize = not (ServerRadioDB.stopOnMinimize ~= false)
        stopOnMinCheck:SetChecked(ServerRadioDB.stopOnMinimize)
        if SetRadioStopOnMinimize then
            SetRadioStopOnMinimize(ServerRadioDB.stopOnMinimize and 1 or 0)
        end
        DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Auto-pause on minimize: " .. (ServerRadioDB.stopOnMinimize and "|cFF00FF00Enabled|r" or "|cFFFF0000Disabled|r"))
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

-- Initialize on enter world
local loaderFrame = CreateFrame("Frame")
loaderFrame:RegisterEvent("PLAYER_ENTERING_WORLD")
loaderFrame:SetScript("OnEvent", function()
    stopOnMinCheck:SetChecked(ServerRadioDB.stopOnMinimize ~= false)
    if SetRadioStopOnMinimize then
        SetRadioStopOnMinimize(ServerRadioDB.stopOnMinimize ~= false and 1 or 0)
    end

    if HasRadioSupport() then
        DEFAULT_CHAT_FRAME:AddMessage("|cFF00FF00[ServerRadio]|r Everlook Broadcasting Co. loaded! Type |cFFFFFF00/radio|r to open player.")
    else
        DEFAULT_CHAT_FRAME:AddMessage("|cFFFF8800[ServerRadio]|r Addon loaded. Type |cFFFFFF00/radio|r.")
    end
end)
