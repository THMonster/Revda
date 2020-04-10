import asyncio
import danmaku
import sys

async def printer(q):
    while True:
        m = await q.get()
        if m['msg_type'] == 'danmaku':
            print(f'[{m["name"]}] {m["content"]}'.encode(sys.stdin.encoding, 'ignore').
                  decode(sys.stdin.encoding))
            sys.stdout.flush()


async def main():
    q = asyncio.Queue()
    dmc = danmaku.DanmakuClient(sys.argv[1], q)
    asyncio.create_task(printer(q))
    await dmc.start()

asyncio.run(main())
