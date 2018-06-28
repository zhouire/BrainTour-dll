cd /d %~dp0

::: default dataset (for debug)
call DefaultSettings.bat

:: VR settings
set AutoDetail=0
set Detail=2
set DetailMotion=2
set WindowW=1024
set WindowH=1024
set TileX=256
set TileY=256

::: local settings
if exist LocalSetting_%COMPUTERNAME%.bat (
	call LocalSetting_%COMPUTERNAME%.bat
)

set RTVRSettings=%1

..\x64\Release\OculusCudaTest.exe