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
            'Referer': 'https://m.twitch.tv/',
            "User-Agent": r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            # 'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        async with aiohttp.ClientSession() as sess:
            async with sess.request("get", self.room_url) as resp:
                t = await resp.text()
                if re.search(r'"User\}\|\{.+?":.+?"stream":{"__ref":', t) == None:
                    raise Exception("no live found!")
                ret["title"] = re.search(
                    r'"BroadcastSettings\}\|\{.+?":.+?"title":"(.+?)"', t
                ).group(1)
                ret["play_url"] = f"https://www.twitch.tv/{self.rid}"

        proc = await asyncio.create_subprocess_shell(
            f"streamlink {ret['play_url']} best --stream-url", stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
        )
        stdout, stderr = await proc.communicate()
        if stdout:
            ret["play_url"] = stdout.decode().strip()

        # print(ret)
        return ret
