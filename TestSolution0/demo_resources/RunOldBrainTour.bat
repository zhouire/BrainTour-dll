cd /d %~dp0

::: default dataset (for debug)
call DefaultSettings.bat

::: 4K w/ mouse settings
set WindowW=3840
set WindowH=2160
set TileX=512
set TileY=512
set ShowUI=0
set AutoRotation=1
set AutoRotationY=10

set AutoDetail=1
set Detail=4
set DetailMotion=2
set RenderScaleMotion=0

::: local settings
if exist LocalSetting_%COMPUTERNAME%.bat (
	call LocalSetting_%COMPUTERNAME%.bat
)

set Settings=%1

..\x64\Release\TestBrainTour.exe

pause