#! /bin/python3
import aiohttp, asyncio, json, sys, re

async def get_title_tw():
    a = re.search(r'twitch.tv/([^/?]+)', sys.argv[1])
    rid = a.group(1)
    u = f'https://m.twitch.tv/{rid}'
    async with aiohttp.ClientSession() as sess:
        async with sess.request('get', u) as resp:
            t = await resp.text()
            if re.search(r'"User\}\|\{.+?":.+?"stream":null', t) != None:
                raise Exception("no live found!")
            title = re.search(r'"BroadcastSettings\}\|\{.+?":.+?"title":"(.+?)"', t).group(1)
            return title, rid

async def get_title_yt():
    a = re.search(r'youtube.com/channel/([^/?]+)', sys.argv[1])
    try:
        cid = a.group(1)
        u = f'https://www.youtube.com/channel/{cid}/videos'
    except:
        a = re.search(r'youtube.com/c/([^/?]+)', sys.argv[1])
        cid = a.group(1)
        u = f'https://www.youtube.com/c/{cid}/videos'
    # print(a.group(1))
    async with aiohttp.ClientSession() as sess:
        async with sess.request('get', u) as resp:
            t = re.search(r'"gridVideoRenderer"[\s\S]+?</script>', await resp.text()).group(0)
            if re.search(r'"gridVideoRenderer".+?"label":"(LIVE|LIVE NOW|PREMIERING NOW)"', t) == None:
                raise Exception("no live found!")
            t = re.search(r'("gridVideoRenderer"(.(?!"gridVideoRenderer"))+"label":"(LIVE|LIVE NOW|PREMIERING NOW)".+)', t).group(1)
            vid = re.search(r'"gridVideoRenderer".+?"videoId":"(.+?)"', t).group(1)
            title = re.search(r'"gridVideoRenderer".+?"title".+?"text":"(.+?)(?<!\\)"', t).group(1)
            return title, vid

async def get_url_yt(vid):
    proc = await asyncio.create_subprocess_shell(
    f'youtube-dl -f best "https://www.youtube.com/watch?v={vid}" -g',
    stdout=asyncio.subprocess.PIPE,
    stderr=asyncio.subprocess.PIPE)
    stdout, stderr = await proc.communicate()
    if stdout:
        return stdout.decode()

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
        title, vid = await get_title_yt()
        url = f'https://www.youtube.com/watch?v={vid}::hls'
    elif "twitch.tv/" in sys.argv[1]:
        title, rid = await get_title_tw()
        url = f'https://www.twitch.tv/{rid}::hls'
    else:
        await run_ykdl(sys.argv[1])
        return

    print("title: " + title, flush=True)
    print(url, flush=True)

asyncio.run(main())
