# Revda
Linux平台下的弹幕直播播放工具。

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
* [just](https://github.com/casey/just) (for build)
* [dioxus-cli](https://crates.io/crates/dioxus-cli) (for build)
* a proper [rust](https://kaisery.github.io/trpl-zh-cn/ch01-01-installation.html) compiler with cargo (for build)
* [mpv](https://github.com/mpv-player/mpv)
* ffmpeg
* webkit2gtk-4.1
* libxdo-dev

## Installation

### From source
```
git clone https://github.com/THMonster/Revda.git --recursive
cd Revda
just build-all
sudo just install-all --prefix-dir /usr
```

## Usage
见[Github Wiki](https://github.com/THMonster/Revda/wiki)
