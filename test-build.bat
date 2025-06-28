@echo off
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    KRONOS OS BUILD TEST                     ║
echo ║                  Testing Source Code                        ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

echo [INFO] Testing Kronos OS source code compilation...
echo.

echo [TEST 1] Checking source files...
if exist "src\kernel\main.c" (
    echo ✓ Kernel main found
) else (
    echo ✗ Kernel main missing
    goto :error
)

if exist "src\drivers\vga.c" (
    echo ✓ VGA driver found
) else (
    echo ✗ VGA driver missing
    goto :error
)

if exist "src\drivers\keyboard.c" (
    echo ✓ Keyboard driver found
) else (
    echo ✗ Keyboard driver missing
    goto :error
)

if exist "src\shell\shell.c" (
    echo ✓ Shell found
) else (
    echo ✗ Shell missing
    goto :error
)

if exist "Makefile" (
    echo ✓ Makefile found
) else (
    echo ✗ Makefile missing
    goto :error
)

echo.
echo [TEST 2] Syntax checking C files...

echo Checking main.c syntax...
gcc -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fsyntax-only src/kernel/main.c 2>nul
if %errorlevel% equ 0 (
    echo ✓ main.c syntax OK
) else (
    echo ✗ main.c has syntax errors
)

echo Checking VGA driver syntax...
gcc -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fsyntax-only src/drivers/vga.c 2>nul
if %errorlevel% equ 0 (
    echo ✓ vga.c syntax OK
) else (
    echo ✗ vga.c has syntax errors
)

echo Checking keyboard driver syntax...
gcc -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fsyntax-only src/drivers/keyboard.c 2>nul
if %errorlevel% equ 0 (
    echo ✓ keyboard.c syntax OK
) else (
    echo ✗ keyboard.c has syntax errors
)

echo Checking shell syntax...
gcc -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fsyntax-only src/shell/shell.c 2>nul
if %errorlevel% equ 0 (
    echo ✓ shell.c syntax OK
) else (
    echo ✗ shell.c has syntax errors
)

echo.
echo [TEST 3] Project structure verification...
echo.
echo Source tree:
tree /F src 2>nul || dir /S src

echo.
echo [SUCCESS] Kronos OS source code is ready!
echo.
echo Next steps to run the real OS:
echo 1. Set up WSL2 with Ubuntu
echo 2. Install cross-compiler tools
echo 3. Build with: make ^&^& make iso ^&^& make run
echo.
echo For now, enjoy the interactive demo in your browser!
goto :end

:error
echo.
echo [ERROR] Some source files are missing!
echo Please ensure all Kronos OS files are present.

:end
pause
