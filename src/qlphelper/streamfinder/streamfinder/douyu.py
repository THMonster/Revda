#! /bin/python3
import aiohttp, asyncio, json, sys, re, pkgutil, random, time, uuid, string
from util.jsengine import JSEngine
from util.match import match1


def get_random_name(l):
    return random.choice(string.ascii_lowercase) + "".join(
        random.sample(string.ascii_letters + string.digits, l - 1)
    )


class Douyu:
    api_url1 = "https://www.douyu.com/swf_api/homeH5Enc"
    api_url2 = "https://open.douyucdn.cn/api/RoomApi/room/"
    api_url3 = "https://www.douyu.com/lapi/live/getH5Play/"
    room_url = ""
    rid = ""

    def __init__(self, url, cookie):
        self.room_url = url
        self.rid = self.room_url.split("/")[-1]
        self.headers = {
            # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            # 'Accept-Encoding': 'gzip, deflate',
            # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
            "Referer": "https://www.douyu.com/",
            "User-Agent": r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            # 'Cookie': cookie
        }
        self.js_md5 = pkgutil.get_data(__name__, "crypto-js-md5.min.js")
        if not isinstance(self.js_md5, str):
            self.js_md5 = self.js_md5.decode()

    async def get_h5enc(self, html):
        js_enc = match1(html, "(var vdwdae325w_64we =[\s\S]+?)\s*</script>")
        if js_enc is None or "ub98484234(" not in js_enc:
            params = {"rids": self.rid}
            async with aiohttp.ClientSession() as sess:
                async with sess.request(
                    "get", self.api_url1, params=params, headers=self.headers
                ) as resp:
                    data = await resp.json()
                    assert data["error"] == 0, data["msg"]
                    js_enc = data["data"]["room" + self.rid]
        return js_enc

    async def ub98484234(self, js_enc, params):
        names_dict = {
            "debugMessages": get_random_name(8),
            "decryptedCodes": get_random_name(8),
            "resoult": get_random_name(8),
            "_ub98484234": get_random_name(8),
            "workflow": match1(js_enc, "function ub98484234\(.+?\Weval\((\w+)\);"),
        }
        js_dom = """
        {debugMessages} = {{{decryptedCodes}: []}};
        if (!this.window) {{window = {{}};}}
        if (!this.document) {{document = {{}};}}
        """.format(
            **names_dict
        )
        js_patch = """
        {debugMessages}.{decryptedCodes}.push({workflow});
        var patchCode = function(workflow) {{
            var testVari = /(\w+)=(\w+)\([\w\+]+\);.*?(\w+)="\w+";/.exec(workflow);
            if (testVari && testVari[1] == testVari[2]) {{
                {workflow} += testVari[1] + "[" + testVari[3] + "] = function() {{return true;}};";
            }}
        }};
        patchCode({workflow});
        var subWorkflow = /(?:\w+=)?eval\((\w+)\)/.exec({workflow});
        if (subWorkflow) {{
            var subPatch = `
                {debugMessages}.{decryptedCodes}.push('sub workflow: ' + subWorkflow);
                patchCode(subWorkflow);
            `.replace(/subWorkflow/g, subWorkflow[1]) + subWorkflow[0];
            {workflow} = {workflow}.replace(subWorkflow[0], subPatch);
        }}
        eval({workflow});
        """.format(
            **names_dict
        )
        js_debug = """
        var {_ub98484234} = ub98484234;
        ub98484234 = function(p1, p2, p3) {{
            try {{
                var resoult = {_ub98484234}(p1, p2, p3);
                {debugMessages}.{resoult} = resoult;
            }} catch(e) {{
                {debugMessages}.{resoult} = e.message;
            }}
            return {debugMessages};
        }};
        """.format(
            **names_dict
        )
        js_enc = js_enc.replace("eval({workflow});".format(**names_dict), js_patch)

        js_ctx = JSEngine()
        js_ctx.append(self.js_md5)
        js_ctx.append(js_dom)
        js_ctx.append(js_enc)
        js_ctx.append(js_debug)

        did = uuid.uuid4().hex
        tt = str(int(time.time()))
        ub98484234 = js_ctx.call("ub98484234", self.rid, did, tt)
        # print(ub98484234)
        # extractor.logger.debug('ub98484234: %s', ub98484234)
        ub98484234 = ub98484234[names_dict["resoult"]]
        params.update(
            {
                "v": match1(ub98484234, "v=(\d+)"),
                "did": did,
                "tt": tt,
                "sign": match1(ub98484234, "sign=(\w{32})"),
            }
        )

    async def get_play_info(self):
        ret = dict()
        title = ""
        artist = ""
        async with aiohttp.ClientSession() as sess:
            async with sess.request("get", self.room_url, headers=self.headers) as resp:
                html = await resp.text()
                self.rid = match1(
                    html,
                    "\$ROOM\.room_id\s*=\s*(\d+)",
                    "room_id\s*=\s*(\d+)",
                    '"room_id.?":(\d+)',
                    "data-onlineid=(\d+)",
                )

                title = match1(html, 'Title-head\w*">([^<]+)<')
                artist = match1(html, 'Title-anchorName\w*" title="([^"]+)"')
                if not title or not artist:
                    async with sess.request(
                        "get", self.api_url2 + self.rid, headers=self.headers
                    ) as resp1:
                        room_data = await resp1.json()
                        if room_data["error"] == 0:
                            room_data = room_data["data"]
                            title = room_data["room_name"]
                            artist = room_data["owner_name"]

        ret["title"] = u"{} - {}".format(title, artist)

        js_enc = await self.get_h5enc(html)
        params = {"cdn": "", "iar": 0, "ive": 0}
        await self.ub98484234(js_enc, params)
        params["rate"] = 0
        live_data = None
        async with aiohttp.ClientSession() as sess:
            async with sess.request(
                "post", self.api_url3 + self.rid, data=params, headers=self.headers
            ) as resp:
                live_data = await resp.json()

        if live_data["error"]:
            print(live_data["msg"])

        live_data = live_data["data"]
        # print(live_data)
        ret["play_url"] = "{}/{}".format(live_data["rtmp_url"], live_data["rtmp_live"])
        # print(ret)
        return ret
