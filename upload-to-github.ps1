# 🚀 Kronos OS GitHub Upload Script (PowerShell)
# This script will upload your Kronos OS project to GitHub

Write-Host "🚀 Kronos OS GitHub Upload Script" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Check if git is installed
try {
    $gitVersion = git --version
    Write-Host "✅ Git is installed: $gitVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ Git is not installed. Please install git first." -ForegroundColor Red
    Write-Host "Download from: https://git-scm.com/download/windows" -ForegroundColor Yellow
    exit 1
}

# Check if we're in the right directory
if (-not (Test-Path "kronos-fixed-demo.html") -and -not (Test-Path "Makefile")) {
    Write-Host "❌ This doesn't appear to be the Kronos OS directory" -ForegroundColor Red
    Write-Host "Please run this script from the Kronos OS project root directory" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ Found Kronos OS project files" -ForegroundColor Green

# Configure git if not already configured
$userName = git config --global user.name
if (-not $userName) {
    Write-Host "⚙️ Configuring Git..." -ForegroundColor Yellow
    git config --global user.name "Sandip Hembram"
    git config --global user.email "sandiphembram2021@gmail.com"
    Write-Host "✅ Git configured" -ForegroundColor Green
}

# Initialize repository if not already done
if (-not (Test-Path ".git")) {
    Write-Host "📁 Initializing git repository..." -ForegroundColor Yellow
    git init
    Write-Host "✅ Repository initialized" -ForegroundColor Green
} else {
    Write-Host "✅ Git repository already exists" -ForegroundColor Green
}

# Create .gitignore if it doesn't exist
if (-not (Test-Path ".gitignore")) {
    Write-Host "📝 Creating .gitignore file..." -ForegroundColor Yellow
    @"
# Build artifacts
*.o
*.bin
*.iso
*.img
*.elf

# IDE files
.vscode/
.idea/
*.swp
*.swo
*~

# OS generated files
.DS_Store
Thumbs.db

# Temporary files
*.tmp
*.log
"@ | Out-File -FilePath ".gitignore" -Encoding UTF8
    Write-Host "✅ .gitignore created" -ForegroundColor Green
}

# Create LICENSE if it doesn't exist
if (-not (Test-Path "LICENSE")) {
    Write-Host "📄 Creating LICENSE file..." -ForegroundColor Yellow
    @"
MIT License

Copyright (c) 2024 Sandip Hembram

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"@ | Out-File -FilePath "LICENSE" -Encoding UTF8
    Write-Host "✅ LICENSE created" -ForegroundColor Green
}

# Use enhanced README if it exists
if ((Test-Path "README_ENHANCED.md") -and -not (Test-Path "README.md")) {
    Write-Host "📖 Using enhanced README..." -ForegroundColor Yellow
    Copy-Item "README_ENHANCED.md" "README.md"
    Write-Host "✅ README updated" -ForegroundColor Green
}

# Add all files
Write-Host "📦 Adding files to git..." -ForegroundColor Yellow
git add .

# Check if there are changes to commit
$status = git status --porcelain
if (-not $status) {
    Write-Host "⚠️ No changes to commit" -ForegroundColor Yellow
} else {
    # Create commit
    Write-Host "💾 Creating commit..." -ForegroundColor Yellow
    $commitMessage = @"
Kronos OS v2.1.0 - Complete Educational Operating System

🚀 Major Features:
- 64-bit kernel with RTOS capabilities
- Modern GUI with desktop environment
- App Store with 15+ applications
- Arduino/IoT integration with hardware commands
- Real-time monitoring and advanced terminal
- Professional file manager with multiple view modes
- Comprehensive settings system (12 categories)
- Enhanced window management with animations
- Educational features for STEM learning

🎯 Applications:
- Terminal with 25+ commands and real-time OS data
- File Manager with grid/list/details views
- Calculator with scientific functions
- Games collection (6 classic games)
- Settings with network, display, and system config
- System Monitor with live hardware metrics
- Music and Video players
- App Store with professional applications

🤖 Arduino/IoT Features:
- GPIO, I2C, SPI, UART, PWM, ADC interfaces
- Environmental sensor monitoring
- Real-time data visualization
- Hardware development tools
- Educational command system

⚡ RTOS Capabilities:
- Priority-based scheduling (100 levels)
- Real-time IPC (semaphores, mutexes, queues)
- Deadline monitoring and performance tracking
- <10μs interrupt latency
- Deterministic behavior for real-time applications

🎓 Educational Focus:
- Perfect for computer science education
- Arduino and embedded systems learning
- STEM education with hands-on experience
- Professional development and research
"@

    git commit -m $commitMessage
    Write-Host "✅ Commit created" -ForegroundColor Green
}

# Add remote if not exists
try {
    $remoteUrl = git remote get-url origin 2>$null
    Write-Host "✅ Remote already exists" -ForegroundColor Green
} catch {
    Write-Host "🔗 Adding GitHub remote..." -ForegroundColor Yellow
    git remote add origin https://github.com/sandiphembram2021/kronos-operating-system.git
    Write-Host "✅ Remote added" -ForegroundColor Green
}

# Set main branch
Write-Host "🌿 Setting main branch..." -ForegroundColor Yellow
git branch -M main

# Push to GitHub
Write-Host "🚀 Pushing to GitHub..." -ForegroundColor Yellow
Write-Host "Note: You may need to enter your GitHub username and Personal Access Token" -ForegroundColor Blue
Write-Host "If you don't have a token, create one at: https://github.com/settings/tokens" -ForegroundColor Blue
Write-Host ""

try {
    git push -u origin main
    Write-Host "✅ Successfully pushed to GitHub!" -ForegroundColor Green
    
    # Create and push tag
    Write-Host "🏷️ Creating release tag..." -ForegroundColor Yellow
    try {
        git tag -a v2.1.0 -m "Kronos OS v2.1.0 - Complete Educational Operating System Release"
        git push origin v2.1.0
        Write-Host "✅ Release tag created" -ForegroundColor Green
    } catch {
        Write-Host "⚠️ Tag already exists or couldn't push tag" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Write-Host "🎉 SUCCESS! Your Kronos OS project has been uploaded to GitHub!" -ForegroundColor Green
    Write-Host ""
    Write-Host "🌐 Repository URL: https://github.com/sandiphembram2021/kronos-operating-system" -ForegroundColor Blue
    Write-Host "🎮 Live Demo: Open kronos-fixed-demo.html in your browser" -ForegroundColor Blue
    Write-Host ""
    Write-Host "📋 Next Steps:" -ForegroundColor Yellow
    Write-Host "1. ⭐ Star your repository on GitHub"
    Write-Host "2. 📝 Add repository description and topics"
    Write-Host "3. 🎮 Enable GitHub Pages for live demos"
    Write-Host "4. 📢 Share with the community"
    Write-Host "5. 🤝 Invite collaborators"
    Write-Host ""
    Write-Host "🌟 Your Kronos OS is now available to the world!" -ForegroundColor Green
    
} catch {
    Write-Host "❌ Failed to push to GitHub" -ForegroundColor Red
    Write-Host ""
    Write-Host "💡 Troubleshooting:" -ForegroundColor Yellow
    Write-Host "1. Make sure you have write access to the repository"
    Write-Host "2. Check your GitHub username and token"
    Write-Host "3. Verify the repository exists: https://github.com/sandiphembram2021/kronos-operating-system"
    Write-Host "4. Try: git remote set-url origin https://YOUR_TOKEN@github.com/sandiphembram2021/kronos-operating-system.git"
    exit 1
}
