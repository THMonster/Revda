# QLivePlayer
Linux平台下的弹幕直播播放器。

第三版发布！

整个项目几乎全部重写，软件的工作方式也完全不同了，性能比上一版更好。

由于上一版用来获取直播弹幕的`danmu`已经多年不更新了，虽然我个人fork了进行简单的维护，但感觉这个项目里面的东西已经过时太久了，所以我自己用aiohttp造了一个轮子[danmaku](https://github.com/IsoaSFlus/danmaku)，不过支持的网站可能并不会太多（懒），目前支持斗鱼、虎牙、B站和YouTube Live。

另外，经过一些事情后我认为，靠streamlink来获取国内直播平台的流并不是太靠谱。这个项目确实非常棒，但并不太适合中国的直播平台。因此在上次斗鱼更新之后，我把针对获取国内直播流的组件换成了[ykdl](https://github.com/zhangn1985/ykdl)，这是一个专注国内视频/直播网站的项目。streamlink用来获取国外直播平台的流，为可选依赖。

同时，增加了一个管理收藏与历史记录的gui前端，使用起来更友好。

## Features
* 稳定60帧的弹幕，无需多言。
* 笔记本风扇不会惨叫（本人i5-7200U，播放时cpu基本工作在最低的700mhz，占用率在10%～20%左右，风扇完全不工作）
* 直播录制，包括直播流和弹幕，弹幕直接包含在录制下来的视频文件中（字幕轨）。
* Bilibili视频的播放与下载（都包含弹幕支持），自动连播以及选集。

## Screenshot
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)

## Requirements
* [mpv](https://github.com/mpv-player/mpv)
* [ykdl](https://github.com/zhangn1985/ykdl)
* ffmpeg
* Qt5(>=5.12)
* cmake
* python-aiohttp
* python3(>=3.7)
* python-protobuf(optional, for YouTube LiveChat support)
* [streamlink](https://streamlink.github.io/)(optional, for foreign streaming service support)

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

## Usage
见[Github Wiki](https://github.com/IsoaSFlus/QLivePlayer/wiki)
