import asyncio
from .dm import danmaku
import sys

async def printer(q):
    while True:
        m = await q.get()
        if m['msg_type'] == 'danmaku':
            print(f'{m.get("color", "ffffff")}[{m["name"]}] {m["content"]}', flush=True)


async def main():
    ex_data = {}
    if len(sys.argv) > 2:
        ex_data['yt_key'] = sys.argv[2]

    q = asyncio.Queue()
    dmc = danmaku.DanmakuClient(sys.argv[1], q, **ex_data)
    asyncio.create_task(printer(q))
    await dmc.start()

def entry():
    asyncio.run(main())

if __name__ == '__main__':
    asyncio.run(main())
