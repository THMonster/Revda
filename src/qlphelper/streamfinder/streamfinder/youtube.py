#! /bin/python3
import aiohttp, asyncio, json, re

class Youtube:
    room_url = ''
    def __init__(self, url, cookie):
        a = re.search(r'youtube.com/channel/([^/?]+)', url)
        try:
            cid = a.group(1)
            self.room_url = f'https://www.youtube.com/channel/{cid}/videos'
        except:
            a = re.search(r'youtube.com/c/([^/?]+)', sys.argv[1])
            cid = a.group(1)
            self.room_url = f'https://www.youtube.com/c/{cid}/videos'
        self.headers = {
            # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            # 'Accept-Encoding': 'gzip, deflate',
            # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
            # 'Referer': 'https://www.bilibili.com/',
            'User-Agent': r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            # 'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        async with aiohttp.ClientSession() as sess:
            async with sess.request('get', self.room_url) as resp:
                t = re.search(r'"gridVideoRenderer"((.(?!"gridVideoRenderer"))(?!"style":"UPCOMING"))+"label":"(LIVE|LIVE NOW|PREMIERING NOW)"([\s\S](?!"style":"UPCOMING"))+?("gridVideoRenderer"|</script>)', await resp.text()).group(0)
                vid = re.search(r'"gridVideoRenderer".+?"videoId":"(.+?)"', t).group(1)
                ret['play_url'] = f'https://www.youtube.com/watch?v={vid}::hls'
                ret['title'] = re.search(r'"gridVideoRenderer".+?"title".+?"text":"(.+?)(?<!\\)"', t).group(1)

        # print(ret)
        return ret
