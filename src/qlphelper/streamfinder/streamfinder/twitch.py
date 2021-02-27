#! /bin/python3
import aiohttp, asyncio, json, re


class Twitch:
    room_url = ""
    rid = ""

    def __init__(self, url, cookie):
        a = re.search(r"twitch.tv/([^/?]+)", url)
        self.rid = a.group(1)
        self.room_url = f"https://m.twitch.tv/{self.rid}"
        self.headers = {
            # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            # 'Accept-Encoding': 'gzip, deflate',
            # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
            # 'Referer': 'https://www.bilibili.com/',
            "User-Agent": r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            # 'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        async with aiohttp.ClientSession() as sess:
            async with sess.request("get", self.room_url) as resp:
                t = await resp.text()
                if re.search(r'"User\}\|\{.+?":.+?"stream":null', t) != None:
                    raise Exception("no live found!")
                ret["title"] = re.search(
                    r'"BroadcastSettings\}\|\{.+?":.+?"title":"(.+?)"', t
                ).group(1)
                ret["play_url"] = f"https://www.twitch.tv/{self.rid}::hls"

        # print(ret)
        return ret
