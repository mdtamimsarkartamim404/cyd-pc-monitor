@echo off
rem ============================================================================
rem  convert_for_cyd.bat  -  any photo / video / gif  ->  files the CYD plays directly
rem  USAGE: drag & drop files (or a whole selection) onto this .bat
rem  NEEDS: ffmpeg in PATH   (Windows: winget install Gyan.FFmpeg)
rem  RESULT: folder "SD" next to the files with  SD\videos  SD\images
rem          -> copy the contents of SD\ to the root of the microSD card
rem ============================================================================
setlocal enabledelayedexpansion
where ffmpeg >nul 2>nul
if errorlevel 1 (
  echo ffmpeg not found. Install it:  winget install Gyan.FFmpeg   then open a new window.
  pause
  exit /b 1
)
if "%~1"=="" (
  echo Drag and drop photo / video / gif files onto this file.
  pause
  exit /b 1
)
set "OUT=%~dp1SD"
mkdir "%OUT%\videos" 2>nul
mkdir "%OUT%\images" 2>nul
mkdir "%OUT%\gifs" 2>nul

rem ---- tweak here ----
set "FPS=15"
rem  quality: -q:v 1 = best ... 31 = worst.  If you see "Frame too big", raise VQ to 5
set "VQ=3"
set "IQ=2"
set "VF=scale=320:240:flags=lanczos:force_original_aspect_ratio=increase,crop=320:240"

:next
if "%~1"=="" goto done
set "n=%~n1"
set "n=!n: =_!"
set "n=!n:(=_!"
set "n=!n:)=_!"
set "e=%~x1"
set "kind="
for %%X in (.mp4 .mkv .avi .mov .webm .m4v .flv .wmv .ts .mpg .mpeg .3gp .gif) do if /I "%%X"=="!e!" set "kind=video"
for %%X in (.jpg .jpeg .png .bmp .webp .tif .tiff .heic .avif) do if /I "%%X"=="!e!" set "kind=image"
if "!kind!"=="video" (
  echo [video] %~nx1
  ffmpeg -v error -y -i "%~1" -vf "%VF%,fps=%FPS%" -c:v mjpeg -pix_fmt yuvj420p -q:v %VQ% -an "%OUT%\videos\!n!.avi"
) else if "!kind!"=="image" (
  echo [image] %~nx1
  ffmpeg -v error -y -i "%~1" -vf "%VF%" -frames:v 1 -c:v mjpeg -pix_fmt yuvj420p -q:v %IQ% "%OUT%\images\!n!.jpg"
) else (
  echo skipped: %~nx1
)
shift
goto next

:done
echo.
echo Done. Copy everything inside "%OUT%" to the microSD card root.
pause
