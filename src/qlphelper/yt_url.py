#! /bin/python3
import aiohttp, asyncio, json, sys, re


async def get_title():
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
            if re.search(r'"gridVideoRenderer".+?"label":"(LIVE|LIVE NOW)"', t) == None:
                raise Exception("no live found!")
            t = re.search(r'("gridVideoRenderer"(.(?!"gridVideoRenderer"))+"label":"(LIVE|LIVE NOW)".+)', t).group(1)
            vid = re.search(r'"gridVideoRenderer".+?"videoId":"(.+?)"', t).group(1)
            title = re.search(r'"gridVideoRenderer".+?"title".+?"text":"(.+?)"', t).group(1)
            return title, vid

async def get_url(vid):
    proc = await asyncio.create_subprocess_shell(
    f'youtube-dl -f best "https://www.youtube.com/watch?v={vid}" -g',
    stdout=asyncio.subprocess.PIPE,
    stderr=asyncio.subprocess.PIPE)
    stdout, stderr = await proc.communicate()
    if stdout:
        return stdout.decode()

async def main():
    title, vid = await get_title()
    # url = await get_url(vid)
    url = f'https://www.youtube.com/watch?v={vid}:qlp-sl'
    print("title: " + title, flush=True)
    print(url, flush=True)


    # async with aiohttp.ClientSession() as sess:
    #     async with sess.request('get', sys.argv[1]) as resp:
    #         a = re.search(r';ytplayer\.config\s*=\s*({.+?});', await resp.text())
    #         b = json.loads(a.group(1))
    #         c = json.loads(b['args']['player_response'])
    #         bestv_rate = 0
    #         bestv_url = ''
    #         besta_rate = 0
    #         besta_url = ''
    #         for st in c['streamingData']['adaptiveFormats']:
    #             if 'video' in st['mimeType']:
    #                 if st['bitrate'] > bestv_rate:
    #                     bestv_rate = st['bitrate']
    #                     bestv_url = st['url']
    #                 else:
    #                     besta_rate = st['bitrate']
    #                     besta_url = st['url']
    #         print(bestv_rate)
    #         print(bestv_url)
    #         print(besta_rate)
    #         print(besta_url)


asyncio.run(main())
