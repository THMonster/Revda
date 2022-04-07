# Revda
Linux平台下的弹幕直播播放工具。

5.0.0发布，更名为Revda，gui工具使用`tauri`重写，不再依赖qt，很多地方有较大改动，请阅读[Github Wiki](https://github.com/THMonster/Revda/wiki)。

## Features
* 支持国内外各大主流直播平台的直播播放。
* 稳定60帧的弹幕。
* 得益于mpv的优秀播放性能。
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
[![Screenshot.png](https://raw.githubusercontent.com/THMonster/Revda/master/pictures/s1.png)](https://raw.githubusercontent.com/THMonster/Revda/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/THMonster/Revda/master/pictures/s2.png)](https://raw.githubusercontent.com/THMonster/Revda/master/pictures/s2.png)

## Requirements
* cmake (for build)
* yarn (for build)
* extra-cmake-modules (for build)
* a proper [rust](https://kaisery.github.io/trpl-zh-cn/ch01-01-installation.html) compiler with cargo (for build)
* [mpv](https://github.com/mpv-player/mpv)
* ffmpeg
* curl
* webkit2gtk
* nodejs (optional, for douyu support)

## Installation

### Archlinux

#### [ArchlinuxCN](https://wiki.archlinux.org/index.php/Unofficial_user_repositories#archlinuxcn) repo

```
sudo pacman -S revda-git
```
Thanks for [@MarvelousBlack](https://github.com/MarvelousBlack)'s maintenance.

#### AUR
```
paru -S revda-git # or any other aur helper
```


### From source
```
git clone https://github.com/THMonster/Revda.git --recursive
cd Revda
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

## Usage
见[Github Wiki](https://github.com/THMonster/Revda/wiki)
