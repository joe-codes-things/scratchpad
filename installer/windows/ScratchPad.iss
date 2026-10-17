#ifndef AppVersion
  #error AppVersion must be provided on the ISCC command line
#endif
#ifndef PluginPath
  #error PluginPath must be provided on the ISCC command line
#endif
#ifndef OutputPath
  #error OutputPath must be provided on the ISCC command line
#endif

[Setup]
AppId={{9DCECE55-3896-43DC-AF5D-AC50D03ECF43}
AppName=ScratchPad
AppVersion={#AppVersion}
AppPublisher=Nine Lives Audio
DefaultDirName={commoncf64}\VST3
MinVersion=10.0.22000
DisableProgramGroupPage=yes
OutputDir={#OutputPath}
OutputBaseFilename=ScratchPad_v{#AppVersion}_Windows
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Uninstallable=yes

[Files]
Source: "{#PluginPath}\*"; DestDir: "{app}\nla_scratchpad_v{#AppVersion}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
