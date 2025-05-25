# 🎯 项目简介

这是一个由SCFG移植而来的项目，旨在将lua语法的lcfg文件编译成适配DearMoments的自动身法文件

---

# 🚀 功能特性

* 支持将自定义脚本编译为 CFG
* 内置多种 SCFG 函数，扩展 Lua 的功能
* 提供平滑的视角控制和动作指令
* 支持自定义 exec 路径设置

---

# 📦 使用方法

1. 将 `lcfg.exe` 和 `lua54.dll` 放置于脚本同目录，或将其添加到系统环境变量中。
2. 使用以下命令编译您的脚本：

   ```bash
   lcfg <your_script_file>
   ```
3. 现在您得到了一个有若干 `cmd_<id>.cfg` 文件的文件夹。将其与 <a href="https://github.com/tomh500/MoClient">DearMoments</a> 一起使用，然后使用以下 cfg 命令来注册到DearMoments按键绑定

   ```bash
   exec <路径>/_init.cfg;
   ```
4. 使用DearMoments的自动身法按键绑定，或者命令```sf_start```进行启动

---

# 📚 支持的 LCFG 函数

在 Lua 语法的基础上，您可以使用以下 LCFG 定义的函数：

* `<basecmd>(数字)`：执行基本动作，如前进、后退、攻击等。对应 CFG 中的 `basecmd x 0 0`。
* `jump()`：执行跳跃动作。
* `sleep()`：以 tick 为单位的延迟。
* `jumpbug()`：执行 Jumpbug 动作。
* `setAngle(yaw, pitch)`：设置视角角度。
* `angle(yaw, pitch)`：在不重置角度的基础上设置视角，避免频繁重置引起的抖动。
* `moveAngle(yaw, pitch, time)`：在指定时间内平滑移动视角到目标角度。
* `src(cmd)`：直接在 CFG 中写入指定的命令内容。
* `setExecPath(path)`：设置生成文件中 exec 的路径。
* 如果您希望锁定鼠标移动，在脚本中使用 `lockMouse()` 和 `unlockMouse()`
* 值得注意的是，LCFG与SCFG脚本并不完全兼容！

---

## 📄 示例

```lua
setExecPath("cfg/autoexec.cfg")

lockMouse()
wasdCancel()

jump()
sleep(5)
setAngle(90, 0)
moveAngle(180, 0, 10)
src("echo Hello, LCFG!")

unlockMouse()
```

---

# 🛠️ 自行编译

1. 确保您的系统已安装 并配置 g++ 环境。
2. 运行以下命令以编译 LCFG：

   ```bash
   g++ ./src/scfg.cpp -I./src/include -L./src/ -llua54 -o lcfg.exe -std=c++23 -static
   ```

---

## 📜 许可证

GPL 3.0

