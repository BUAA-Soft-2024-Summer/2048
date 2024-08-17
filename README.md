# 2048 Game Demo

> Copyright &copy; Tony's Studio 2024

---

## 编译运行

在 Visual Studio（推荐 Visual Studio 2022）中打开项目，构建解决方案或工程即可。

<img src="README/image-20240817115118811.png" alt="image-20240817115118811" style="zoom:50%;" />

构建时，可以选择配置（Configuration），有 Debug 和 Release 两种。

<img src="README/image-20240817115205027.png" alt="image-20240817115205027" style="zoom:50%;" />

最后，点击运行即可，也可以跳过前两步，直接运行。左侧为调试运行（可以打断点），右侧为非调试运行。

<img src="README/image-20240817115259027.png" alt="image-20240817115259027" style="zoom:50%;" />

## 打包发布

> 这一步需要你本地有 [Inno Setup](https://jrsoftware.org/isinfo.php)。

**在 Release 配置下**，项目有“构建后事件”（Post-Build Event），会将可执行文件和图标拷贝至安装脚本目录（`install\`）。

<img src="README/image-20240817115456757.png" alt="image-20240817115456757" style="zoom:50%;" />

接下来，打开 `install` 目录中的 `Setup.iss` 脚本，点击 Build > Compile 编译构建我们的安装包。

<img src="README/image-20240817115812714.png" alt="image-20240817115812714" style="zoom:50%;" />

构建完成后，即可在 `install\` 目录中得到 `2048 Setup.exe`，即我们的安装包。

<img src="README/image-20240817120104575.png" alt="image-20240817120104575" style="zoom:50%;" />