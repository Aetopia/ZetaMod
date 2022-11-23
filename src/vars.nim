import steam
import os

let
  steampath = getSteamPath() 
  localappdata* = getEnv("LOCALAPPDATA")
  gamedir* = getSteamGameInstallDir("Halo Infinite", steampath)
  dxgiini* = gamedir/"dxgi.ini"
  gameconfig* = localappdata/"HaloInfinite/Settings/SpecControlSettings.json"
  steamclient* = steampath/"steam.exe"
  documents* = getEnv("USERPROFILE")/"Documents"
  temp* = getEnv("TEMP")
  bwdmthook* = "\n[Import.BWDMTHook]\nArchitecture=x64\nFilename=BWDMTHook.dll\nRole=ThirdParty\nWhen=Early"
  bwdmt* = gamedir/"BWDMT.txt"
  xml* = """<?xml version="1.0" encoding="UTF-16"?>
<Task version="1.4" xmlns="http://schemas.microsoft.com/windows/2004/02/mit/task">
  <RegistrationInfo>
    <Date>2022-09-28T17:40:33.9968206</Date>
    <Author>ZetaConfig</Author>
    <URI>\ResEnforce</URI>
  </RegistrationInfo>
  <Triggers>
    <LogonTrigger>
      <Enabled>true</Enabled>
    </LogonTrigger>
  </Triggers>
  <Principals>
    <Principal id="Author">
      <UserId>$1</UserId>
      <LogonType>InteractiveToken</LogonType>
      <RunLevel>LeastPrivilege</RunLevel>
    </Principal>
  </Principals>
  <Settings>
    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>
    <AllowHardTerminate>false</AllowHardTerminate>
    <StartWhenAvailable>true</StartWhenAvailable>
    <RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable>
    <IdleSettings>
      <StopOnIdleEnd>false</StopOnIdleEnd>
      <RestartOnIdle>false</RestartOnIdle>
    </IdleSettings>
    <AllowStartOnDemand>true</AllowStartOnDemand>
    <Enabled>true</Enabled>
    <Hidden>false</Hidden>
    <RunOnlyIfIdle>false</RunOnlyIfIdle>
    <DisallowStartOnRemoteAppSession>false</DisallowStartOnRemoteAppSession>
    <UseUnifiedSchedulingEngine>true</UseUnifiedSchedulingEngine>
    <WakeToRun>false</WakeToRun>
    <ExecutionTimeLimit>PT0S</ExecutionTimeLimit>
    <Priority>4</Priority>
  </Settings>
  <Actions Context="Author">
    <Exec>
      <Command>"$2"</Command>
    </Exec>
  </Actions>
</Task>"""