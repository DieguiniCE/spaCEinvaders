@echo off
REM Compila cliente jugador y espectador.
REM Usa la libreria raylib incluida en el repo.

set RAYLIB_PATH=%~dp0lib
set CC=gcc
set CFLAGS=-Wall -std=c17 -I. -Iinclude
set LIBS=-L%RAYLIB_PATH% -lraylib -lws2_32 -lopengl32 -lgdi32 -lwinmm

cd /d "%~dp0"

echo Compilando cliente jugador...
%CC% %CFLAGS% main.c serial.c ListaAliens.c %LIBS% -o cliente.exe
if %ERRORLEVEL% NEQ 0 exit /b 1

echo Compilando cliente espectador...
%CC% %CFLAGS% espectador.c ListaAliens.c %LIBS% -o espectador.exe
if %ERRORLEVEL% NEQ 0 exit /b 1

echo Listo: cliente.exe [COMx]  |  espectador.exe
exit /b 0
