# QLivePlayer
Linux平台下的弹幕直播播放器
## Screenshots
[![Screenshot_20171117_221638.png](https://i.loli.net/2017/11/18/5a0f94bf383cc.png)](https://i.loli.net/2017/11/18/5a0f94bf383cc.png)




























## Requirements
* [libmpv](https://github.com/mpv-player/mpv)
* [streamlink](https://github.com/streamlink/streamlink)
* [danmu](https://github.com/littlecodersh/danmu)
* Qt5
* qmake
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
  -h, --help             Displays this help.
  -v, --version          Displays version information.
  -u, --url <url>        The Live Stream url to open.
  -s, --stream <stream>  The stream to open, default is "best".
```
Example:
`qliveplayer  -u https://www.douyu.com/2550505 -s source_http`
## Keyboard Control
* `d` 显示/隐藏弹幕
* `f` 全屏/窗口化
* `q` 退出
* `space` 暂停/播放
* `-/=` 音量减/加
* `m` 静音/取消静音
## Support Sites
取决于上游的[streamlink](https://github.com/streamlink/streamlink)以及[danmu](https://github.com/littlecodersh/danmu)，两者的交集就是支持的直播平台。
