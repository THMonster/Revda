#! /bin/python3
import aiohttp, asyncio, json, sys, re
from bilibili import Bilibili
from douyu import Douyu
from huya import Huya
from youtube import Youtube
from twitch import Twitch

async def run_ykdl(url):
    proc = await asyncio.create_subprocess_shell(
    f'ykdl -i {url}',
    stdout=asyncio.subprocess.PIPE,
    stderr=asyncio.subprocess.PIPE)
    stdout, stderr = await proc.communicate()
    if stdout:
        print(stdout.decode(), flush=True)
    if stderr:
        print(stderr.decode(), flush=True)

async def main():
    if "youtube.com/c" in sys.argv[1]:
        s = Youtube(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        data = await s.get_play_info()
        title = data['title']
        url = data['play_url']
    elif "twitch.tv/" in sys.argv[1]:
        s = Twitch(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        data = await s.get_play_info()
        title = data['title']
        url = data['play_url']
    elif "douyu.com/" in sys.argv[1]:
        s = Douyu(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        data = await s.get_play_info()
        title = data['title']
        url = data['play_url']
    elif "huya.com/" in sys.argv[1]:
        s = Huya(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        data = await s.get_play_info()
        title = data['title']
        url = data['play_url']
    elif "live.bilibili.com/" in sys.argv[1]:
        s = Bilibili(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        data = await s.get_play_info()
        title = data['title']
        url = data['play_url']
    elif "bilibili.com/" in sys.argv[1]:
        s = Bilibili(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
        await s.get_play_info_video()
        return
    else:
        await run_ykdl(sys.argv[1])
        return

    print("title: " + title, flush=True)
    print(url, flush=True)

def entry():
    asyncio.run(main())

if __name__ == '__main__':
    asyncio.run(main())
