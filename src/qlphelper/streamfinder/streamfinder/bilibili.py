#! /bin/python3
import aiohttp, asyncio, json, sys, re
from .util.match import match1

class Bilibili:
    api_url1 = 'https://api.live.bilibili.com/xlive/web-room/v2/index/getRoomPlayInfo'
    api_url2 = 'https://api.live.bilibili.com/xlive/web-room/v1/index/getInfoByRoom'
    api_url_v = 'https://api.bilibili.com/x/player/playurl'
    api_url_v_ep = 'https://api.bilibili.com/pgc/player/web/playurl'
    room_url = ''
    def __init__(self, url, cookie):
        self.room_url = url
        self.headers = {
            # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            # 'Accept-Encoding': 'gzip, deflate',
            # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
            'Referer': 'https://www.bilibili.com/',
            'User-Agent': r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36",
            'Cookie': cookie
        }

    async def get_play_info(self):
        ret = dict()
        rid = self.room_url.split('/')[-1]
        params1 = {
            'room_id': rid,
            'no_playurl': 0,
            'mask': 1,
            'qn': 10000,
            'platform': 'web',
            'protocol': '0,1',
            'format': '0,2',
            'codec': '0,1'
        }
        params2 = {
            'room_id': rid,
        }
        async with aiohttp.ClientSession() as sess:
            async with sess.request('get', self.api_url1, params=params1, headers=self.headers) as resp:
                j = await resp.json()
                j = j['data']['playurl_info']['playurl']['stream'][0]['format'][0]['codec'][0]
                ret['play_url'] = j['url_info'][0]['host'] + j['base_url'] + j['url_info'][0]['extra']
            async with sess.request('get', self.api_url2, params=params2) as resp:
                j = await resp.json()
                ret['title'] = j['data']['room_info']['title'] + ' - ' + j['data']['anchor_info']['base_info']['uname']

        # print(ret)
        return ret

    async def get_page_info(self, url):
        page_index = match1(url, '\?p=(\d+)', 'index_(\d+)\.') or '1'
        data = None
        async with aiohttp.ClientSession() as session:
            async with session.request('get', url, headers=self.headers) as r:
                data = json.loads(match1(await r.text(), '__INITIAL_STATE__=({.+?});'))['videoData']
        bvid = data['bvid']
        title = data['title']
        artist = data['owner']['name']
        pages = data['pages']
        for page in pages:
            index = str(page['page'])
            subtitle = page['part']
            if index == page_index:
                cid = page['cid']
                if len(pages) > 1:
                    title = u'{} - {} - {}'.format(title, index, subtitle)
                elif subtitle and subtitle != title:
                    title = u'{} - {}'.format(title, subtitle)
                break

        return bvid, cid, title, artist

    async def get_page_info_ep(self, url):
        async with aiohttp.ClientSession() as session:
            async with session.request('get', url, headers=self.headers) as r:
                data = json.loads(match1(await r.text(), '__INITIAL_STATE__=({.+?});'))
        title = data.get('h1Title') or match1(html, '<title>(.+?)_番剧_bilibili_哔哩哔哩<')
        cid = data['epInfo']['cid']
        bvid = data['epInfo']['bvid']
        mediaInfo = data['mediaInfo']
        season_type = mediaInfo.get('season_type') or mediaInfo.get('ssType')
        upInfo = mediaInfo.get('upInfo')
        artist = upInfo and upInfo.get('name')

        return bvid, cid, title, artist, season_type

    async def get_play_info_video(self):
        bili_url = self.room_url
        real_url = []
        dash_id = 0
        dash_urls = []
        durl_id = 0
        durl_urls = []
        title = ''
        artist = ''
        if 'bilibili.com/bangumi' in bili_url:
            bvid, cid, title, artist, season_type = await self.get_page_info_ep(bili_url)
            params = {
                'cid': cid,
                'bvid': bvid,
                'qn': 120,
                'otype': 'json',
                'fourk': 1,
                'fnver': 0,
                'fnval': 16
            }
            async with aiohttp.ClientSession() as session:
                async with session.request('get', self.api_url_v_ep, params=params, headers=self.headers) as r:
                    j = await r.json()
                    # print(j)
                    if "dash" in j["result"]:
                        dash_id = j['result']['dash']['video'][0]['id']
                        if len(j['result']['dash']['video']) > 1 and  dash_id == j['result']['dash']['video'][1]['id']:
                            # 12 = hevc 7 = avc
                            if j['result']['dash']['video'][0]['codecid'] != 12:
                                dash_urls.append(j['result']['dash']['video'][0]['base_url'])
                                dash_urls.append(j['result']['dash']['audio'][0]['base_url'])
                                dash_urls.append(j['result']['dash']['video'][1]['base_url'])
                            else:
                                dash_urls.append(j['result']['dash']['video'][1]['base_url'])
                                dash_urls.append(j['result']['dash']['audio'][0]['base_url'])
                                dash_urls.append(j['result']['dash']['video'][0]['base_url'])
                        else:
                            dash_urls.append(j['result']['dash']['video'][0]['base_url'])
                            dash_urls.append(j['result']['dash']['audio'][0]['base_url'])
                    elif 'durl' in j['result']:
                        for u in j['result']['durl']:
                            dash_urls.append(u['url'])
        else:
            bvid, cid, title, artist = await self.get_page_info(bili_url)
            params = {
                'cid': cid,
                'bvid': bvid,
                'qn': 120,
                'otype': 'json',
                'fourk': 1,
                'fnver': 0,
                'fnval': 16
            }
            async with aiohttp.ClientSession() as session:
                async with session.request('get', self.api_url_v, params=params, headers=self.headers) as r:
                    j = await r.json()
                    # print(j)
                    if "dash" in j["data"]:
                        dash_id = j['data']['dash']['video'][0]['id']
                        if len(j['data']['dash']['video']) > 1 and dash_id == j['data']['dash']['video'][1]['id']:
                            # 12 = hevc 7 = avc
                            if j['data']['dash']['video'][0]['codecid'] != 12:
                                dash_urls.append(j['data']['dash']['video'][0]['base_url'])
                                dash_urls.append(j['data']['dash']['audio'][0]['base_url'])
                                dash_urls.append(j['data']['dash']['video'][1]['base_url'])
                            else:
                                dash_urls.append(j['data']['dash']['video'][1]['base_url'])
                                dash_urls.append(j['data']['dash']['audio'][0]['base_url'])
                                dash_urls.append(j['data']['dash']['video'][0]['base_url'])
                        else:
                            dash_urls.append(j['data']['dash']['video'][0]['base_url'])
                            dash_urls.append(j['data']['dash']['audio'][0]['base_url'])
                    elif 'durl' in j['data']:
                        for u in j['data']['durl']:
                            dash_urls.append(u['url'])

        print("title:" + title)
        for u in dash_urls:
            print(u, flush=True)
