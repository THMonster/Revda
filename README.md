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
* 录制，包括直播流和弹幕，弹幕直接包含在录制下来的视频文件中（字幕轨）。

## Usage
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)

编译安装之后在终端启动`qliveplayer`可以打开gui。

当想要加入一个直播房间时，在左上角输入房间代码，房间代码的格式为`<prefix>-<roomid>`prefix的规则为直播网站域名的前两个字母，如：`do`、`hu`、`bi`。举个例子，如果你想进斗鱼的9999号房间，那么房间代码就是`do-9999`。输入完成后按回车会旁边的键头就会打开mpv进行播放（会缓冲几秒，不要暴躁狂点，否则会弹出一堆mpv的窗口）。侧边栏的第二个按钮是历史记录页面，会记录下你的播放历史，点爱心图标便可收藏，不过需要注意，为了防止手滑，收藏/取消收藏的操作只有在点击右上的刷新按钮（按f5也能刷新，取决于你的DE）之后才会真正应用。点击爱心旁边的锁链图标可以快捷打开该直播间进行播放，同样需要注意，不要狂点，mpv会缓冲一段时间，点多了就会弹一堆mpv出来。如果你等了很久都没有出来播放窗口，建议用下面的cli程序手动输入链接看看能不能成功播放，有没有输出有用的信息。

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
  --no-window          No window if specified, useful for recording
  -d, --debug          Show debug info

```
### Control
为了最好的效率，这个版本不再使用libmpv，而是直接调用mpv本身，所以在播放时会吃你自己对mpv的设置，播放时的控制也和原生的mpv一样，参考[interactive-control](https://mpv.io/manual/stable/#interactive-control)

## Tips
* 由于ass是根据视频的每一帧来渲染的，所以在播放录制下来的视频文件时为了追求最好的动画效果，推荐把视频帧率上采样至60fps播放，这里提供一个基于mpv的使用ffmpeg内建滤波器的方法 :
`mpv  --vf 'lavfi="fps=fps=60:round=down"' <your-video-file>`

* 同时播放的直播并没有数量限制，你可以同时打开多个直播间，且每个播放进程和gui前端进程是分离的，退出gui前端不会关闭正在播放的直播。

### Record stream

在`qlphelper`中使用`-r`参数可以录制直播流到指定的文件。录制下来的文件使用matroska容器封装，包含三轨，分别是视频、音频和字幕轨，弹幕包含在字幕轨中。当只使用`-r`参数进行录制，会同时进行播放，如果你只想录制不想同时播放，那么可以在使用`-r`参数的同时加上`--no-window`参数，便不会同时播放。如果在加了`--no-window`之后又想边录边看了怎么办？有两种方法，第一种是直接再开个`qlphelper`看，这样做自然会占用两带宽；第二种是直接用mpv播放录制文件，这样不会占用额外的带宽也不会影响录制。

另外我还提供了一个简单的脚本`qlprecorder`方便进行无人值守的录制，启动后它会按时轮询自动重试。这个脚本会一并安装，不需要额外下载，具体使用方法可以自己看一看，就不多说了。


## Requirements
* [mpv](https://github.com/mpv-player/mpv)
* [ykdl](https://github.com/zhangn1985/ykdl)
* ffmpeg
* Qt5(>=5.4)
* cmake
* python-aiohttp
* python3(>=3.7)

## Installation

### Archlinux

Thanks for [@MarvelousBlack](https://github.com/MarvelousBlack)'s maintenance.

#### [ArchlinuxCN](https://wiki.archlinux.org/index.php/Unofficial_user_repositories#archlinuxcn) repo

```
sudo pacman -S kcm-colorful-git
```
#### AUR
```
yay -S kcm-colorful-git # or any other aur helper
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

