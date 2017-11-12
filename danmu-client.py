#!/bin/python3
import time, sys
import threading

from danmu import DanMuClient


def pp(msg):
    print(msg)
    sys.stdout.flush()


dmc = DanMuClient(sys.argv[1])
if not dmc.isValid(): 
    print('Url not valid')
    sys.exit()

@dmc.danmu
def danmu_fn(msg):
    pp('[%s] %s' % (msg['NickName'], msg['Content']))

dmc.start(blockThread = True)
