# QLivePlayer
Linux平台下的弹幕直播播放器。

## Features
* 支持国内外各大主流直播平台的直播播放。
* 稳定60帧的弹幕，无需多言。
* 笔记本风扇不会惨叫（本人i5-7200U，播放时cpu基本工作在最低的700mhz，占用率在10%～20%左右，风扇完全不工作）
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
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s1.png)
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/pictures/s2.png)

## Requirements
* extra-cmake-modules (for build)
* [mpv](https://github.com/mpv-player/mpv)
* ffmpeg
* curl
* Qt5 (>=5.12)
* cmake
* python-aiohttp
* python3 (>=3.7)
* python-protobuf (optional, for YouTube LiveChat support)
* [streamlink](https://streamlink.github.io/) (optional, for foreign streaming service support)

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
