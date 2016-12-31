echo INFO: Running PostBuild.bat
REM below is an example of running an cmd line exe within this script
REM start "CMD Window Title" /wait "TestSomething.exe"

echo Copying Engine.lib
xcopy /y /f build\Engine.lib lib\
echo INFO: PostBuild.bat Complete
