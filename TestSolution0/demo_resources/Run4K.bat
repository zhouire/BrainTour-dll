cd /d %~dp0

::: default dataset (for debug)
call DefaultSettings.bat

::: 4K settings
set WindowW=3840
set WindowH=2160
set TileX=512
set TileY=512

set NoHead=1
set AutoDetail=1
set Detail=3
set DetailMotion=1
set Gamma=1.0

::: local settings
if exist LocalSetting_%COMPUTERNAME%.bat (
	call LocalSetting_%COMPUTERNAME%.bat
)

set RTVRSettings=%1

..\x64\Release\OculusCudaTest.exe
