# QTCIDE - Professional Qt IDE

A modern, professional Qt-based Integrated Development Environment with advanced features and glassmorphic UI design.

## Features

- 🎨 **Modern Glassmorphic UI** - Beautiful, modern interface design
- 🔧 **Multi-Terminal Support** - PowerShell, CMD, Bash, Zsh, Fish support
- 📁 **Project Management** - Create and manage Qt projects easily
- 🔨 **Integrated Build System** - CMake and Ninja integration
- 📝 **Advanced Code Editor** - Syntax highlighting and code completion
- 🌍 **Cross-Platform** - Windows, macOS, and Linux support
- ⚡ **Fast Performance** - Optimized for speed and efficiency

## Installation

### Windows
- Download the MSI installer or portable ZIP
- Run the installer and follow the setup wizard
- Or extract the portable version to any folder

### macOS
- Download the DMG file
- Drag QTCIDE to Applications folder

### Linux
- Download the DEB package: `sudo dpkg -i qtcide_1.0.0_amd64.deb`
- Or download the RPM package: `sudo rpm -i qtcide-1.0.0.x86_64.rpm`

## Building from Source

### Prerequisites
- Qt 6.0 or later
- CMake 3.16 or later
- C++17 compatible compiler
- Ninja build system (optional but recommended)

### Build Instructions

```bash
git clone https://github.com/qtcide/qtcide.git
cd qtcide
mkdir build
cd build
cmake .. -G Ninja
cmake --build .
```

### Create Installer Packages

```bash
# Build and create all packages
cmake --build . --target bundle-all
cmake --build . --target package

# Windows specific
cmake --build . --target deploy          # Deploy Qt libraries
cmake --build . --target package-portable # Create portable package
```

## Usage

1. **Create a New Project**: File → New Project
2. **Open Existing Project**: File → Open Project
3. **Configure Build**: Build → Configure
4. **Build Project**: Build → Build (Ctrl+B)
5. **Run Application**: Build → Run (Ctrl+R)

## Terminal Configuration

QTCIDE automatically detects available terminals on your system:

- **Windows**: cmd, PowerShell, PowerShell Core, MSYS2, MinGW64, Git Bash
- **macOS**: zsh, bash, fish
- **Linux**: bash, zsh, fish, dash

Configure your preferred terminal in Tools → Settings.

## Requirements

- **Windows**: Windows 10 or later
- **macOS**: macOS 10.15 or later
- **Linux**: Any modern distribution with Qt 6 support

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

## Support

- GitHub Issues: https://github.com/qtcide/qtcide/issues
- Documentation: https://qtcide.github.io/docs
- Community: https://discord.gg/qtcide

## Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.
