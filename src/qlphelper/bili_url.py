#! /bin/python3
# import request
import hashlib, json, re, sys
from xml.dom.minidom import parseString
import asyncio, aiohttp
from urllib.parse import urlencode

api_url = 'https://api.bilibili.com/x/player/playurl'
api_url_ep = 'https://api.bilibili.com/pgc/player/web/playurl'

# test_url = 'https://www.bilibili.com/video/BV1Gt4y117MH'
test_url = 'https://www.bilibili.com/video/BV1xs411R7qf'

headers = {
    # 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
    # 'Accept-Encoding': 'gzip, deflate',
    # 'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
    'Referer': 'https://www.bilibili.com/',
    'User-Agent': r"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36"
}

def match1(text, *patterns):
    """Scans through a string for substrings matched some patterns (first-subgroups only).
    Args:
        text: A string to be scanned.
        patterns: Arbitrary number of regex patterns.
    Returns:
        When matches, returns first-subgroups from first match.
        When no matches, return None
    """

    for pattern in patterns:
        try:
            match = re.search(pattern, text)
        except(TypeError):
            match = re.search(pattern, str(text))
        if match:
            return match.group(1)
    return None

async def get_page_info(url):
    page_index = match1(url, '\?p=(\d+)', 'index_(\d+)\.') or '1'
    data = None
    async with aiohttp.ClientSession() as session:
        async with session.request('get', url, headers=headers) as r:
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

async def get_page_info_ep(url):
    async with aiohttp.ClientSession() as session:
        async with session.request('get', url, headers=headers) as r:
            data = json.loads(match1(await r.text(), '__INITIAL_STATE__=({.+?});'))
    title = data.get('h1Title') or match1(html, '<title>(.+?)_番剧_bilibili_哔哩哔哩<')
    cid = data['epInfo']['cid']
    bvid = data['epInfo']['bvid']
    mediaInfo = data['mediaInfo']
    season_type = mediaInfo.get('season_type') or mediaInfo.get('ssType')
    upInfo = mediaInfo.get('upInfo')
    artist = upInfo and upInfo.get('name')

    return bvid, cid, title, artist, season_type

async def main():
    if len(sys.argv) > 2:
        headers['Cookie'] = sys.argv[2]
    else:
        headers['Cookie'] = ''

    bili_url = sys.argv[1]
    real_url = []
    dash_id = 0
    dash_urls = []
    durl_id = 0
    durl_urls = []
    title = ''
    artist = ''
    if 'bilibili.com/bangumi' in bili_url:
        bvid, cid, title, artist, season_type = await get_page_info_ep(bili_url)
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
            async with session.request('get', api_url_ep, params=params, headers=headers) as r:
                j = await r.json()
                print(j)
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
        bvid, cid, title, artist = await get_page_info(bili_url)
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
            async with session.request('get', api_url, params=params, headers=headers) as r:
                j = await r.json()
                print(j)
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
        print(u)

asyncio.run(main())
