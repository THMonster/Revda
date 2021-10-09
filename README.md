# QLivePlayer
Linux平台下的弹幕直播播放器。

2021/07/18: 4.0.0发布，编译过程与之前相比有所变动，主要在于4.0.0版本将之前版本中使用python实现的功能全部用rust重构，故不再依赖python与python相关的组件，但在编译时需要用户安装有rust编译器。

## Features
* 支持国内外各大主流直播平台的直播播放。
* 稳定60帧的弹幕。
* 笔记本风扇不会惨叫（在i5-7200U下播放时cpu基本工作在最低的700mhz，占用率在10%～20%左右，风扇完全不工作）
* 直播录制，包括直播流和弹幕，弹幕直接包含在录制下来的视频文件中（字幕轨）。
* Bilibili视频的播放与下载（都包含弹幕支持），自动连播以及选集。

## Support Status

|    Site    |   直播   |    直播弹幕    |   视频   |  视频弹幕   |
|:----------:|:----------:|:----------:|:----------:|:----------:|
| Bilibili | ✓ | ✓ | ✓ | ✓ |
| Douyu | ✓ | ✓ | |  |
| Huya | ✓ | ✓ | | |
| YouTube | ✓ | ✓ | | |
| Twitch | ✓ | ✓ | | |

## Screenshot
[![Screenshot.png](https://raw.githubusercontent.com/THMonster/QLivePlayer/master/pictures/s1.png)](https://raw.githubusercontent.com/THMonster/QLivePlayer/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/THMonster/QLivePlayer/master/pictures/s2.png)](https://raw.githubusercontent.com/THMonster/QLivePlayer/master/pictures/s2.png)

## Requirements
* cmake (for build)
* extra-cmake-modules (for build)
* a proper [rust](https://kaisery.github.io/trpl-zh-cn/ch01-01-installation.html) compiler with cargo (for build)
* [mpv](https://github.com/mpv-player/mpv)
* ffmpeg
* curl
* Qt5 (>=5.15)

## Installation

### Archlinux

```
sudo pacman -S qliveplayer
```

#### [ArchlinuxCN](https://wiki.archlinux.org/index.php/Unofficial_user_repositories#archlinuxcn) repo

```
sudo pacman -S qliveplayer-git
```
Thanks for [@MarvelousBlack](https://github.com/MarvelousBlack)'s maintenance.

#### AUR
```
yay -S qliveplayer-git # or any other aur helper
```

### Gentoo

#### [GURU](https://wiki.gentoo.org/wiki/Project:GURU)
Thanks for [@jian-lin](https://github.com/jian-lin)'s maintenance, and check [this](https://github.com/THMonster/QLivePlayer/issues/14#issuecomment-739154154) for details.

### From source
```
git clone https://github.com/THMonster/QLivePlayer.git --recursive
cd QLivePlayer
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

## Usage
见[Github Wiki](https://github.com/THMonster/QLivePlayer/wiki)
