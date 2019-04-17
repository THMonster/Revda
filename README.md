# QLivePlayer
Linux平台下的弹幕直播播放器。
进入低维护状态（不会添加新功能（~~好像也没什么可以加的了~~））,原因大概是我对性能还是不是非常满意，尝试新的性能更好的方法。。。不过这个坑就有点大了，我也不知道什么时候会填。

## Features
* 60 fps！
* 带弹幕支持的直播播放功能。
* 直播流录制。
* 超超超实用的弹幕录制功能！！！将弹幕数据转换成ASS文件，可以在任何主流视频播放器（推荐使用[mpv](https://mpv.io)）下配合录制的视频食用，回看也有弹幕的欢乐～

## Tips
* 由于ass是根据视频的每一帧来渲染的，所以为了追求最好的动画效果，推荐把录制的视频帧率上采样至60fps来配合ass文件，这里提供一个基于mpv的使用ffmpeg内建滤波器的方法 :
`mpv  --vf 'lavfi="fps=fps=60:round=down"' <your-video-file>`

## Screenshots
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/screenshot.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/screenshot.png)




























## Requirements
* [libmpv](https://github.com/mpv-player/mpv)
* [streamlink](https://github.com/streamlink/streamlink)
* [danmu](https://github.com/littlecodersh/danmu)
* Qt5(>=5.4)
* cmake
* Python3

## Installation
### From source
```
cd /path/to/source
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

## Usage
```
Usage: qliveplayer [options]
A cute and useful Live Stream Player with danmaku support.
Project address: https://github.com/IsoaSFlus/QLivePlayer

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

观看的同时录制直播流以及弹幕 `qliveplayer  -u https://www.douyu.com/2550505 -s source_http --record-stream ~/out.flv --record-danmaku ~/out.ass`

仅录制 `qliveplayer  -u https://www.douyu.com/2550505 -s source_http --record-stream ~/out.flv --record-danmaku ~/out.ass -c`


## Keyboard Control
* `d` 显示/隐藏弹幕
* `f` 全屏/窗口化
* `q` 退出
* `space` 暂停/播放
* `-/=` 音量减/加
* `m` 静音/取消静音
* `r` 刷新

## Support Sites
取决于上游的[streamlink](https://github.com/streamlink/streamlink)以及[danmu](https://github.com/littlecodersh/danmu)，两者的交集就是支持的直播平台。

## Known Issues
* 为了兼容性以及性能原因，我把弹幕直接渲染在了视频的每一帧上，而mpv为了保持同步似乎会让渲染函数阻塞线程，这就导致若要使弹幕以60fps渲染，就需要让视频也是60fps。我目前找到了两个方法，各有利弊：第一种方法是直接指定mpv参数，设定display-fps为60fps后指定video-sync为display-resample，便可上采样至60fps。这种方法cpu占用较低，不过一旦因为网络或其他原因使直播流不稳定会触发mpv的保护机制，使播放帧率重新与视频原始帧率一致，这时候你会发现弹幕突然变得超级慢。。。于是我加了一个刷新功能，按r刷新后就又正常了。第二种方法与上面tips中的方法原理是一致的，就是使用ffmpeg内建的滤波器，这种方法占用cpu会高一些，但好处是没有第一种方法的问题，更为稳定。目前使用第一种方法，因为随着mpv的更新第一种方法的稳定性有所提升。

