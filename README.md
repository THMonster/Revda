# QLivePlayer
Linux平台下的弹幕直播播放器。
### version 1.12(2017.11.27)
* 优化弹幕显示策略
### version 1.1(2017.11.26)
* 重写弹幕显示策略
* 加入直播流录制功能
* 加入弹幕录制功能
## Features
* 带弹幕支持的直播播放功能
* 直播流录制
* 超超超实用的弹幕录制功能！！！将弹幕数据转换成ASS文件，可以在任何主流视频播放器（推荐使用[mpv](https://mpv.io)）下配合录制的视频食用，回看也有弹幕的欢乐～
## Screenshots
[![Screenshot_20171117_221638.png](https://i.loli.net/2017/11/18/5a0f94bf383cc.png)](https://i.loli.net/2017/11/18/5a0f94bf383cc.png)




























## Requirements
* [libmpv](https://github.com/mpv-player/mpv)
* [streamlink](https://github.com/streamlink/streamlink)
* [danmu](https://github.com/littlecodersh/danmu)
* Qt5
* qmake
* Python3
## Installation
### From source
```
$ cd /path/to/source
$ qmake
$ make
```
## Usage
```
Usage: qliveplayer [options]
A cute and useful Live Stream Player with danmaku support.

Options:
  -h, --help                   Displays this help.
  -v, --version                Displays version information.
  -u, --url <url>              The Live Stream url to open.
  -s, --stream <stream>        The stream to open, default is "best".
  --record-stream <filepath>   File to save recorded stream
  --record-danmaku <filepath>  File to save recorded danmaku
  -c, --without-gui            CLI mode, available only with recording.

```
Example:

直接观看 `qliveplayer  -u https://www.douyu.com/2550505 -s source_http`

观看的同时录制直播流以及弹幕`qliveplayer  -u https://www.douyu.com/2550505 -s source_http --record-stream ~/out.flv --record-danmaku ~/out.ass`

仅录制`qliveplayer  -u https://www.douyu.com/2550505 -s source_http --record-stream ~/out.flv --record-danmaku ~/out.ass -c`

## Keyboard Control
* `d` 显示/隐藏弹幕
* `f` 全屏/窗口化
* `q` 退出
* `space` 暂停/播放
* `-/=` 音量减/加
* `m` 静音/取消静音
## Support Sites
取决于上游的[streamlink](https://github.com/streamlink/streamlink)以及[danmu](https://github.com/littlecodersh/danmu)，两者的交集就是支持的直播平台。
