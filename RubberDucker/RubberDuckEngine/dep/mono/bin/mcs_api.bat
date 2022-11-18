@echo off
"%~dp0\mono.exe" %MONO_OPTIONS% "%~dp0\..\lib\mono\4.5\mcs.exe" %* -t:library -out:RDEScriptsAPI.dll source\mono\apiscripts\*.cs > mcs_api_output.txt 2>&1
