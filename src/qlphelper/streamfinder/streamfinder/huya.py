#! /bin/python3
import aiohttp, asyncio, json, sys, re, random, base64
from html.parser import HTMLParser
from util.match import match1

class Huya:
    api_url1 = 'https://www.douyu.com/swf_api/homeH5Enc'
    api_url2 = 'https://open.douyucdn.cn/api/RoomApi/room/'
    api_url3 = 'https://www.douyu.com/lapi/live/getH5Play/'
    room_url = ''
    rid = ''
    def __init__(self, url, cookie):
        self.room_url = url
        self.rid = self.room_url.split('/')[-1]
        self.headers = {
            # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            # 'Accept-Encoding': 'gzip, deflate',
            # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
            'Referer': 'https://www.huya.com/',
            'User-Agent': r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            # 'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        title = ''
        artist = ''

        async with aiohttp.ClientSession() as sess:
            async with sess.request('get', self.room_url, headers=self.headers) as resp:
                html = await resp.text()
                json_stream = match1(html, '"stream": "([a-zA-Z0-9+=/]+)"')
                assert json_stream, "live video is offline"
                data = json.loads(base64.b64decode(json_stream).decode())
                assert data['status'] == 200, data['msg']

                room_info = data['data'][0]['gameLiveInfo']
                ret['title'] = u'{}「{} - {}」'.format(
                    room_info['roomName'], room_info['nick'], room_info['introduction'])

                stream_info = random.choice(data['data'][0]['gameStreamInfoList'])
                sStreamName = stream_info['sStreamName']
                for sType in ('flv', 'hls'):
                    sType = sType.title()
                    sUrl = stream_info['s{}Url'.format(sType)]
                    sUrlSuffix = stream_info['s{}UrlSuffix'.format(sType)]
                    sAntiCode = stream_info['s{}AntiCode'.format(sType)]
                    ret['play_url'] = HTMLParser().unescape(u'{}/{}.{}?{}'.format(sUrl, sStreamName, sUrlSuffix, sAntiCode))
                    break

        # print(ret)
        return ret
