#! /bin/python3
import aiohttp, asyncio, json, re
from urllib.parse import parse_qsl


class Youtube:
    room_url = ""

    def __init__(self, url, cookie):
        a = re.search(r"youtube.com/channel/([^/?]+)", url)
        try:
            self.cid = a.group(1)
            self.room_url = f"https://www.youtube.com/channel/{self.cid}/videos"
        except:
            a = re.search(r"youtube.com/watch\?v=([^/?]+)", url)
            self.room_url = f"https://www.youtube.com/embed/{a.group(1)}"
        self.headers = {
            "Referer": "https://www.youtube.com/",
            "User-Agent": r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            "Accept-Language": "en-US",
            # 'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        vid = ""
        hls_manifest = ""
        async with aiohttp.ClientSession() as sess:
            if "youtube.com/embed/" in self.room_url:
                async with sess.request("get", self.room_url, headers=self.headers) as resp:
                    b = re.search(r'\\"channelId\\":\\"(.{24})\\"', await resp.text())
                    self.cid = b.group(1)
                    self.room_url = f"https://www.youtube.com/channel/{self.cid}/videos"
            async with sess.request("get", self.room_url, headers=self.headers) as resp:
                t = re.search(
                    r'"gridVideoRenderer"((.(?!"gridVideoRenderer"))(?!"style":"UPCOMING"))+"label":"(LIVE|LIVE NOW|PREMIERING NOW)"([\s\S](?!"style":"UPCOMING"))+?("gridVideoRenderer"|</script>)',
                    await resp.text(),
                ).group(0)
                vid = re.search(r'"gridVideoRenderer".+?"videoId":"(.+?)"', t).group(1)
                ret["play_url"] = f"https://www.youtube.com/watch?v={vid}"
                ret["title"] = re.search(
                    r'"gridVideoRenderer".+?"title".+?"text":"(.+?)(?<!\\)"', t
                ).group(1)
            p = {
                'video_id': vid,
                'html5': 1,
                'el': 'detailpage'
            }
            # async with sess.request("get", f"https://www.youtube.com/get_video_info", params=p, headers=self.headers) as resp:
            async with sess.request("get", f"https://www.youtube.com/watch?v={vid}", headers=self.headers) as resp:
                hls_manifest = re.search(r'"hlsManifestUrl":"([^"]+)"', await resp.text()).group(1)
                # print(hls_manifest)
                # ret["play_url"] = j["streamingData"]["adaptiveFormats"][0]["url"]
            async with sess.request("get", hls_manifest) as resp:
                for line in (await resp.text()).splitlines():
                    if line.startswith("http"):
                        ret["play_url"] = line.strip()

        # print(ret)
        return ret
