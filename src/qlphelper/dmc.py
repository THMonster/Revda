import asyncio, sys
import danmaku

def cb(m):
    if m['msg_type'] == 'danmaku':
        print(f'[{m["name"]}] {m["content"]}'.encode(sys.stdin.encoding, 'ignore').
              decode(sys.stdin.encoding))
        sys.stdout.flush()


async def main():
    dmc = danmaku.DanmakuClient(sys.argv[1], cb)
    await dmc.start()

asyncio.run(main())
