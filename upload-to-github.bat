@echo off
title Kronos OS GitHub Upload

echo.
echo 🚀 Kronos OS GitHub Upload Script
echo ==================================
echo.

REM Check if git is installed
git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Git is not installed. Please install git first.
    echo Download from: https://git-scm.com/download/windows
    pause
    exit /b 1
)

echo ✅ Git is installed

REM Check if we're in the right directory
if not exist "kronos-fixed-demo.html" if not exist "Makefile" (
    echo ❌ This doesn't appear to be the Kronos OS directory
    echo Please run this script from the Kronos OS project root directory
    pause
    exit /b 1
)

echo ✅ Found Kronos OS project files

REM Configure git if not already configured
for /f "tokens=*" %%i in ('git config --global user.name 2^>nul') do set username=%%i
if "%username%"=="" (
    echo ⚙️ Configuring Git...
    git config --global user.name "Sandip Hembram"
    git config --global user.email "sandiphembram2021@gmail.com"
    echo ✅ Git configured
)

REM Initialize repository if not already done
if not exist ".git" (
    echo 📁 Initializing git repository...
    git init
    echo ✅ Repository initialized
) else (
    echo ✅ Git repository already exists
)

REM Create .gitignore if it doesn't exist
if not exist ".gitignore" (
    echo 📝 Creating .gitignore file...
    (
        echo # Build artifacts
        echo *.o
        echo *.bin
        echo *.iso
        echo *.img
        echo *.elf
        echo.
        echo # IDE files
        echo .vscode/
        echo .idea/
        echo *.swp
        echo *.swo
        echo *~
        echo.
        echo # OS generated files
        echo .DS_Store
        echo Thumbs.db
        echo.
        echo # Temporary files
        echo *.tmp
        echo *.log
    ) > .gitignore
    echo ✅ .gitignore created
)

REM Use enhanced README if it exists
if exist "README_ENHANCED.md" if not exist "README.md" (
    echo 📖 Using enhanced README...
    copy "README_ENHANCED.md" "README.md" >nul
    echo ✅ README updated
)

REM Add all files
echo 📦 Adding files to git...
git add .

REM Create commit
echo 💾 Creating commit...
git commit -m "Kronos OS v2.1.0 - Complete Educational Operating System"

REM Add remote if not exists
git remote get-url origin >nul 2>&1
if %errorlevel% neq 0 (
    echo 🔗 Adding GitHub remote...
    git remote add origin https://github.com/sandiphembram2021/kronos-operating-system.git
    echo ✅ Remote added
) else (
    echo ✅ Remote already exists
)

REM Set main branch
echo 🌿 Setting main branch...
git branch -M main

REM Push to GitHub
echo 🚀 Pushing to GitHub...
echo Note: You may need to enter your GitHub username and Personal Access Token
echo If you don't have a token, create one at: https://github.com/settings/tokens
echo.

git push -u origin main
if %errorlevel% equ 0 (
    echo ✅ Successfully pushed to GitHub!
    
    REM Create and push tag
    echo 🏷️ Creating release tag...
    git tag -a v2.1.0 -m "Kronos OS v2.1.0 Release"
    git push origin v2.1.0
    
    echo.
    echo 🎉 SUCCESS! Your Kronos OS project has been uploaded to GitHub!
    echo.
    echo 🌐 Repository URL: https://github.com/sandiphembram2021/kronos-operating-system
    echo 🎮 Live Demo: Open kronos-fixed-demo.html in your browser
    echo.
    echo 📋 Next Steps:
    echo 1. ⭐ Star your repository on GitHub
    echo 2. 📝 Add repository description and topics
    echo 3. 🎮 Enable GitHub Pages for live demos
    echo 4. 📢 Share with the community
    echo 5. 🤝 Invite collaborators
    echo.
    echo 🌟 Your Kronos OS is now available to the world!
) else (
    echo ❌ Failed to push to GitHub
    echo.
    echo 💡 Troubleshooting:
    echo 1. Make sure you have write access to the repository
    echo 2. Check your GitHub username and token
    echo 3. Verify the repository exists
    echo 4. Try using Personal Access Token instead of password
)

echo.
pause
