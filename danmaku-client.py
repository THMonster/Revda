import time, sys
from danmu import DanMuClient
def pp(msg):
    print(msg.encode(sys.stdin.encoding, 'ignore').
        decode(sys.stdin.encoding))
    sys.stdout.flush()
dmc = DanMuClient(sys.argv[1])
if not dmc.isValid(): 
    print('Unsupported danmu server')
    sys.exit()
@dmc.danmu
def danmu_fn(msg):
    pp('[%s] %s' % (msg['NickName'], msg['Content']))
dmc.start(blockThread = True)
