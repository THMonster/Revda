# QLivePlayer
Linux平台下的弹幕直播播放器。

第三版发布！

整个项目几乎全部重写，软件的工作方式也完全不同了，性能比上一版更好。

由于上一版用来获取直播弹幕的`danmu`已经多年不更新了，虽然我个人fork了进行简单的维护，但感觉这个项目里面的东西已经过时太久了，所以我自己用aiohttp造了一个轮子[danmaku](https://github.com/IsoaSFlus/danmaku)，不过支持的网站可能并不会太多（懒），目前只支持斗鱼、虎牙与B站。因此，本项目完整支持的网站目前只有这三家，也就是说其他的直播网站可能可以播放，但暂时没有弹幕支持。

另外，经过一些事情我发现，靠streamlink来获取直播流并不是太靠谱，这个项目确实非常棒，但并不太适合中国的直播网站。因此在上次斗鱼更新之后，我把获取直播流的组件换成了[ykdl](https://github.com/zhangn1985/ykdl)，这是一个专注国内视频/直播网站的项目，相对来说更可靠。

## Features
* 稳定60帧的弹幕，无需多言。
* 笔记本风扇不会惨叫（本人i5-7200U，播放时cpu基本工作在最低的700mhz，占用率在10%～20%左右，风扇完全不工作）
* 录制，包括直播流和弹幕，弹幕直接包含在录制下来的视频文件中（字幕轨）。

## Usage
```
qlphelper -h
Usage: qlphelper [options]

Options:
  -h, --help           Displays help on commandline options
  --help-all           Displays help including Qt specific options
  -u, --url <url>      The Live Stream url to open
  -r, --record <file>  Record stream to local file
  -d, --debug          Show debug info

```
### Control
为了最好的效率，这个版本不再使用libmpv，而是直接调用mpv本身，所以在播放时会吃你自己对mpv的设置，播放时的控制也和原生的mpv一样，参考[interactive-control](https://mpv.io/manual/stable/#interactive-control)

## Tips
* 由于ass是根据视频的每一帧来渲染的，所以在播放录制下来的视频文件时为了追求最好的动画效果，推荐把视频帧率上采样至60fps播放，这里提供一个基于mpv的使用ffmpeg内建滤波器的方法 :
`mpv  --vf 'lavfi="fps=fps=60:round=down"' <your-video-file>`

## Screenshots
[![Screenshot.png](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/screenshot.png)](https://raw.githubusercontent.com/IsoaSFlus/QLivePlayer/master/screenshot.png)




























## Requirements
* [mpv](https://github.com/mpv-player/mpv)
* [danmaku](https://github.com/IsoaSFlus/danmaku)
* [ykdl](https://github.com/zhangn1985/ykdl)
* ffmpeg
* Qt5(>=5.4)
* cmake
* python3(>=3.7)

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




## Known Issues
* 某些带emoji的弹幕会导致画面变卡，当然等那条弹幕过去之后就会恢复，你也可以直接按`j`关闭再开启弹幕把当前的弹幕刷掉。

