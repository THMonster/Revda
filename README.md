# QLivePlayer
Linux平台下的弹幕直播播放器。

第三版发布！

整个项目几乎全部重写，软件的工作方式也完全不同了，性能比上一版更好。

由于上一版用来获取直播弹幕的`danmu`已经多年不更新了，虽然我个人fork了进行简单的维护，但感觉这个项目里面的东西已经过时太久了，所以我自己用aiohttp造了一个轮子[danmaku](https://github.com/IsoaSFlus/danmaku)，不过支持的网站可能并不会太多（懒），目前只支持斗鱼、虎牙与B站。因此，本项目完整支持的网站目前只有这三家，也就是说其他的直播网站可能可以播放，但暂时没有弹幕支持。

另外，经过一些事情我发现，靠streamlink来获取直播流并不是太靠谱，这个项目确实非常棒，但并不太适合中国的直播网站。因此在上次斗鱼更新之后，我把获取直播流的组件换成了[ykdl](https://github.com/zhangn1985/ykdl)，这是一个专注国内视频/直播网站的项目，相对来说更可靠。

同时，增加了一个管理收藏与历史记录的gui前端，使用起来更友好。

## Features
* 稳定60帧的弹幕，无需多言。
* 笔记本风扇不会惨叫（本人i5-7200U，播放时cpu基本工作在最低的700mhz，占用率在10%～20%左右，风扇完全不工作）
* 直播录制，包括直播流和弹幕，弹幕直接包含在录制下来的视频文件中（字幕轨）。
* Bilibili视频的播放与下载（都包含弹幕支持）,自动连播以及选集。

## Usage
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)

编译安装之后在终端启动`qliveplayer`可以打开gui。

当想要加入一个直播房间时，在左上角输入房间代码，房间代码的格式为`<prefix>-<roomid>`prefix的规则为直播网站域名的前两个字母，如：`do`、`hu`、`bi`。举个例子，如果你想进斗鱼的9999号房间，那么房间代码就是`do-9999`。输入完成后按回车会旁边的键头就会打开mpv进行播放（会缓冲几秒，不要暴躁狂点，否则会弹出一堆mpv的窗口）。侧边栏的第二个按钮是历史记录页面，会记录下你的播放历史，点爱心图标便可收藏，不过需要注意，为了防止手滑，收藏/取消收藏的操作只有在点击右上的刷新按钮（按f5也能刷新，取决于你的DE）之后才会真正应用。点击爱心旁边的锁链图标可以快捷打开该直播间进行播放，同样需要注意，不要狂点，mpv会缓冲一段时间，点多了就会弹一堆mpv出来。如果你等了很久都没有出来播放窗口，建议用下面的cli程序手动输入链接看看能不能成功播放，有没有输出有用的信息。

Bilibili视频支持输入av号、bv号、ep号、ss号或直接输入链接，多p视频如果想通过av、bv号或者ss号打开，请在编号后加上`:n`（n为视频p数）打开，如：你想打开av123456的第三p，请输入`av123456:3`，bv号与ss号同理。另外，对于多p视频以及包含多集的番剧当一p/一集播放结束后会进行自动连播，同时也可以在mpv的控制台中输入如`script-message qlpgo-<int>`直接转跳到你想看的集数/p数。针对有大会员限制的视频，可以通过添加cookie实现访问。具体操作方法只需要在配置文件（一般在`${XDG_CONFIG_HOME}/QLivePlayer/QLivePlayer.conf`)中加上一行如`bcookie="<your-bilibili-cookie>"`。**在使用此项功能前，请您注意：本项目不会采集任何相关数据，但访问大会员限制视频将会携带身份向bilibili发起请求，任何潜在的风险，本项目概不负责！**。另外，现版本加入了对B站4k、hevc的支持。4k本身需要大会员才能观看，所以需要有大会员身份的cookie。而hevc需在前面提到的配置文件中添加一行`bhevc=true`，如此方会优先选择hevc流（在有hevc流的前提下），否则将默认选择avc流。另外一些老的低热度的视频可能没有dash流，此时需要有登录过的cookie才能拿到720p及以上的清晰度。同时，`qlphelper`的`-r`参数可以对b站的视频进行下载，`-r`参数指定的文件推荐使用`.mkv`后缀，因为只有mkv才支持内挂ass。

当前版本还添加了直接从网站中打开的功能（暂时只支持Bilibili视频）。原理是通过xdg-open，事先在系统中注册`qliveplayer://`这个protocol的处理方法，然后通过浏览器脚本选择要播放的网页中的目标。必要的两个文件都放在了项目的`misc`目录下，其中`open-with-qliveplayer.user.js`通过tampermonkey或者同类软件安装即可；`qlp-mime.desktop`则安装至`~/.local/share/applications`下，之后刷新mime缓存即可。具体的使用方法是，将鼠标移动至你想通过qliveplayer打开的视频上，然后按下`alt+q`再动一动鼠标。如果是想播放“该页面”，那么对“空白处”进行前述操作即可。

cli程序用法如下：

```
qlphelper -h
Usage: qlphelper [options]

Options:
  -h, --help           Displays help on commandline options.
  --help-all           Displays help including Qt specific options.
  -v, --version        Displays version information.
  -u, --url <url>      The Live Stream url to open
  -r, --record <file>  Record stream to local file
  -d, --debug          Show debug info

```
### Control
为了最好的效率，这个版本不再使用libmpv，而是直接调用mpv本身，所以在播放时会吃你自己对mpv的设置，播放时的控制也和原生的mpv一样，参考[interactive-control](https://mpv.io/manual/stable/#interactive-control)

## Tips
* 由于ass是根据视频的每一帧来渲染的，所以在播放录制下来的视频文件时为了追求最好的动画效果，推荐把视频帧率上采样至60fps播放，这里提供一个基于mpv的使用ffmpeg内建滤波器的方法 :
`mpv  --vf 'lavfi="fps=fps=60:round=down"' <your-video-file>`

* 同时播放的直播并没有数量限制，你可以同时打开多个直播间，且每个播放进程和gui前端进程是分离的，退出gui前端不会关闭正在播放的直播。

* 遇到网络波动导致直播断开会自动重连，不需要重启整个程序。同时也支持手动刷新，在mpv中打开控制台（如何打开[见此](https://mpv.io/manual/master/#keyboard-control))，输入`script-message qlpreload`便可以进行手动刷新。

### Record stream

在`qlphelper`中使用`-r`参数可以录制直播流到指定的文件，文件名会包含一个自动添加的数字后缀（增序）以防止覆盖。录制下来的文件使用matroska容器封装，包含三轨，分别是视频、音频和字幕轨，弹幕包含在字幕轨中。

另外我还提供了一个简单的脚本`qlprecorder`方便进行无人值守的录制，启动后它会按时轮询自动重试。这个脚本会一并安装，不需要额外下载，具体使用方法可以自己看一看，就不多说了。


## Requirements
* [mpv](https://github.com/mpv-player/mpv)
* [ykdl](https://github.com/zhangn1985/ykdl)
* ffmpeg
* Qt5(>=5.12)
* cmake
* python-aiohttp
* python3(>=3.7)

## Installation

### Archlinux

Thanks for [@MarvelousBlack](https://github.com/MarvelousBlack)'s maintenance.

#### [ArchlinuxCN](https://wiki.archlinux.org/index.php/Unofficial_user_repositories#archlinuxcn) repo

```
sudo pacman -S qliveplayer-git
```
#### AUR
```
yay -S qliveplayer-git # or any other aur helper
```

### From source
```
git clone https://github.com/IsoaSFlus/QLivePlayer.git
cd QLivePlayer
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```




## Known Issues
* 某些带emoji的弹幕会导致画面变卡，当然等那条弹幕过去之后就会恢复，你也可以直接按`j`关闭再开启弹幕把当前的弹幕刷掉（当前版本已经屏蔽了emoji，会在弹幕中以`[em]`显示）。
* 录制下来的文件不带index，如有需要，使用ffmpeg重新混流即可。之所以不让在录制时写trailer是因为我曾多次碰到ffmpeg在录制结束后写trailer把整个视频文件写坏的情况，为了录制数据的安全，只能稍微麻烦一点了。
* 某些b站的bv号实际映射到的是番剧，但转跳并非通过http而是通过js实现的，这一暂时无解，所以会无法通过该bv号播放，解决方法就是通过浏览器找到实际的ep号来播放。
* `open-with-qliveplayer`脚本暂时无法针对某些iframe中的内容生效（比如b站的那个下拉动态）。
