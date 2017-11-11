#!/bin/python3
import time, sys
import threading

from danmu import DanMuClient

danmaku_temp = str((time.time())) + '\n'

def pp(msg):
    print(msg.encode(sys.stdin.encoding, 'ignore').
        decode(sys.stdin.encoding))

def append_danmaku(danmaku):
    global danmaku_temp
    #print(danmaku)
    danmaku_temp += danmaku + '\n'

def write_temp_danmaku():
    global danmaku_temp
    f = open('/tmp/danmaku.temp', 'w')
    f.write(danmaku_temp)
    f.close()
    danmaku_temp = str((time.time())) + '\n'
    global timer
    timer = threading.Timer(2, write_temp_danmaku)
    timer.start()

dmc = DanMuClient('http://www.douyu.com/58428')
if not dmc.isValid(): print('Url not valid')

@dmc.danmu
def danmu_fn(msg):
    append_danmaku('%s' % (msg['Content']))
    #pp('%s' % (msg['Content']))

timer = threading.Timer(2, write_temp_danmaku)
timer.start()
dmc.start(blockThread = True)
