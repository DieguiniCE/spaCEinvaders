@echo off
cd /d "%~dp0"
if not exist bin mkdir bin
javac -encoding UTF-8 -d bin *.java
if %ERRORLEVEL% NEQ 0 exit /b 1
echo Servidor compilado. Ejecutar: java -cp bin Servidor.server
exit /b 0
