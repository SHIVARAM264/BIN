@echo off
echo.
echo  ============================================
echo   CleanCycle - Starting Backend Server...
echo  ============================================
echo.
cd /d "%~dp0"
where node >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Node.js not found! Download from https://nodejs.org
  pause
  exit /b
)
IF NOT EXIST node_modules (
  echo [INFO] Installing dependencies...
  npm install
)
echo [INFO] Starting server on http://localhost:3000
echo [INFO] Press Ctrl+C to stop
echo.
start "" "http://localhost:3000"
node server.js
pause
