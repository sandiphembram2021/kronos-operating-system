#!/bin/bash

# 🚀 Kronos OS GitHub Upload Script
# This script will upload your Kronos OS project to GitHub

echo "🚀 Kronos OS GitHub Upload Script"
echo "=================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo -e "${RED}❌ Git is not installed. Please install git first.${NC}"
    echo "Ubuntu/Debian: sudo apt install git"
    echo "Windows: Download from https://git-scm.com/"
    exit 1
fi

echo -e "${GREEN}✅ Git is installed${NC}"

# Check if we're in the right directory
if [ ! -f "kronos-fixed-demo.html" ] && [ ! -f "Makefile" ]; then
    echo -e "${RED}❌ This doesn't appear to be the Kronos OS directory${NC}"
    echo "Please run this script from the Kronos OS project root directory"
    exit 1
fi

echo -e "${GREEN}✅ Found Kronos OS project files${NC}"

# Configure git if not already configured
if [ -z "$(git config --global user.name)" ]; then
    echo -e "${YELLOW}⚙️ Configuring Git...${NC}"
    git config --global user.name "Sandip Hembram"
    git config --global user.email "sandiphembram2021@gmail.com"
    echo -e "${GREEN}✅ Git configured${NC}"
fi

# Initialize repository if not already done
if [ ! -d ".git" ]; then
    echo -e "${YELLOW}📁 Initializing git repository...${NC}"
    git init
    echo -e "${GREEN}✅ Repository initialized${NC}"
else
    echo -e "${GREEN}✅ Git repository already exists${NC}"
fi

# Create .gitignore if it doesn't exist
if [ ! -f ".gitignore" ]; then
    echo -e "${YELLOW}📝 Creating .gitignore file...${NC}"
    cat > .gitignore << 'EOF'
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
EOF
    echo -e "${GREEN}✅ .gitignore created${NC}"
fi

# Create LICENSE if it doesn't exist
if [ ! -f "LICENSE" ]; then
    echo -e "${YELLOW}📄 Creating LICENSE file...${NC}"
    cat > LICENSE << 'EOF'
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
EOF
    echo -e "${GREEN}✅ LICENSE created${NC}"
fi

# Use enhanced README if it exists
if [ -f "README_ENHANCED.md" ] && [ ! -f "README.md" ]; then
    echo -e "${YELLOW}📖 Using enhanced README...${NC}"
    cp README_ENHANCED.md README.md
    echo -e "${GREEN}✅ README updated${NC}"
fi

# Add all files
echo -e "${YELLOW}📦 Adding files to git...${NC}"
git add .

# Check if there are changes to commit
if git diff --staged --quiet; then
    echo -e "${YELLOW}⚠️ No changes to commit${NC}"
else
    # Create commit
    echo -e "${YELLOW}💾 Creating commit...${NC}"
    git commit -m "Kronos OS v2.1.0 - Complete Educational Operating System

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
- Professional development and research"

    echo -e "${GREEN}✅ Commit created${NC}"
fi

# Add remote if not exists
if ! git remote get-url origin &> /dev/null; then
    echo -e "${YELLOW}🔗 Adding GitHub remote...${NC}"
    git remote add origin https://github.com/sandiphembram2021/kronos-operating-system.git
    echo -e "${GREEN}✅ Remote added${NC}"
else
    echo -e "${GREEN}✅ Remote already exists${NC}"
fi

# Set main branch
echo -e "${YELLOW}🌿 Setting main branch...${NC}"
git branch -M main

# Push to GitHub
echo -e "${YELLOW}🚀 Pushing to GitHub...${NC}"
echo -e "${BLUE}Note: You may need to enter your GitHub username and Personal Access Token${NC}"
echo -e "${BLUE}If you don't have a token, create one at: https://github.com/settings/tokens${NC}"
echo ""

if git push -u origin main; then
    echo -e "${GREEN}✅ Successfully pushed to GitHub!${NC}"
    
    # Create and push tag
    echo -e "${YELLOW}🏷️ Creating release tag...${NC}"
    if git tag -a v2.1.0 -m "Kronos OS v2.1.0 - Complete Educational Operating System Release"; then
        if git push origin v2.1.0; then
            echo -e "${GREEN}✅ Release tag created${NC}"
        else
            echo -e "${YELLOW}⚠️ Tag already exists or couldn't push tag${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️ Tag already exists${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}🎉 SUCCESS! Your Kronos OS project has been uploaded to GitHub!${NC}"
    echo ""
    echo -e "${BLUE}🌐 Repository URL: https://github.com/sandiphembram2021/kronos-operating-system${NC}"
    echo -e "${BLUE}🎮 Live Demo: Open kronos-fixed-demo.html in your browser${NC}"
    echo ""
    echo -e "${YELLOW}📋 Next Steps:${NC}"
    echo "1. ⭐ Star your repository on GitHub"
    echo "2. 📝 Add repository description and topics"
    echo "3. 🎮 Enable GitHub Pages for live demos"
    echo "4. 📢 Share with the community"
    echo "5. 🤝 Invite collaborators"
    echo ""
    echo -e "${GREEN}🌟 Your Kronos OS is now available to the world!${NC}"
    
else
    echo -e "${RED}❌ Failed to push to GitHub${NC}"
    echo ""
    echo -e "${YELLOW}💡 Troubleshooting:${NC}"
    echo "1. Make sure you have write access to the repository"
    echo "2. Check your GitHub username and token"
    echo "3. Verify the repository exists: https://github.com/sandiphembram2021/kronos-operating-system"
    echo "4. Try: git remote set-url origin https://YOUR_TOKEN@github.com/sandiphembram2021/kronos-operating-system.git"
    exit 1
fi
