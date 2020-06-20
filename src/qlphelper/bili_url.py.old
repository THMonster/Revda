#! /bin/python3
# import request
import hashlib, json, re, sys
from xml.dom.minidom import parseString
import asyncio, aiohttp
from urllib.parse import urlencode

APPKEY = 'iVGUTjsxvpLeuDCf'
SECRETKEY = 'aHRmhWMLkdeMuILqORnYZocwMBpMEOdt'
APPKEY1 = '84956560bc028eb7' # Bangumi
SECRETKEY1 = '94aba54af9065f71de72f5508f1cd42e'
api_url = 'https://interface.bilibili.com/v2/playurl'
api_url1 = 'https://bangumi.bilibili.com/player/web_api/v2/playurl'
token_url = "https://api.bilibili.com/x/player/playurl/token"

# test_url = 'https://www.bilibili.com/video/BV1Gt4y117MH'
test_url = 'https://www.bilibili.com/video/BV1xs411R7qf'

headers = {
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
    'Accept-Encoding': 'gzip, deflate',
    'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0 Iceweasel/38.2.1'
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
    aid = data['aid']
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

    return aid, cid, title, artist

async def get_page_info1(url):
    async with aiohttp.ClientSession() as session:
        async with session.request('get', url, headers=headers) as r:
            data = json.loads(match1(await r.text(), '__INITIAL_STATE__=({.+?});'))
    title = data.get('h1Title') or match1(html, '<title>(.+?)_番剧_bilibili_哔哩哔哩<')
    cid = data['epInfo']['cid']
    aid = data['epInfo']['aid']
    mediaInfo = data['mediaInfo']
    season_type = mediaInfo.get('season_type') or mediaInfo.get('ssType')
    upInfo = mediaInfo.get('upInfo')
    artist = upInfo and upInfo.get('name')

    return aid, cid, title, artist, season_type

def sign_api_url(api_url, params_str, skey):
    chksum = hashlib.md5(bytes(params_str + skey, 'utf8')).hexdigest()
    return '{}?{}&sign={}'.format(api_url, params_str, chksum)

def get_api_url(cid, qn=200):
    params_str = urlencode([
        ('appkey', APPKEY),
        ('cid', cid),
        ('otype', 'json'),
        ('platform', 'html5'),
        ('player', 0),
        ('qn', qn)
    ])
    return sign_api_url(api_url, params_str, SECRETKEY)

def get_api_url1(cid, season_type, qn=116):
    params_str = urlencode([
        ('appkey', APPKEY),
        ('cid', cid),
        ('module', 'bangumi'),
        ('otype', 'json'),
        ('qn', qn),
        ('quality', qn),
        ('season_type', season_type),
        ('type', '')
    ])
    return sign_api_url(api_url1, params_str, SECRETKEY)

async def get_utoken(aid, cid):
    utoken = ''
    params = {
        'aid': aid,
        'cid': cid
    }
    async with aiohttp.ClientSession() as session:
        async with session.request('get', token_url, headers=headers, params=params) as r:
            try:
                j = await r.json()
                utoken = j['data'].get('token', '')
            except:
                pass
    return utoken

async def main():
    if len(sys.argv) > 2:
        headers['Cookie'] = sys.argv[2]
    else:
        headers['Cookie'] = ''

    bili_url = sys.argv[1]
    real_url = []
    title = ''
    artist = ''
    if 'bilibili.com/bangumi' in bili_url:
        aid, cid, title, artist, season_type = await get_page_info1(bili_url)
        qn = 0
        url = get_api_url1(cid, season_type, qn)
        async with aiohttp.ClientSession() as session:
            async with session.request('get', url, headers=headers) as r:
                j = await r.json()
                qn = j['accept_quality'][0]
        url = get_api_url1(cid, season_type, qn)
        async with aiohttp.ClientSession() as session:
            async with session.request('get', url, headers=headers) as r:
                j = await r.json()
                # print(j)
                for u in j['durl']:
                    real_url.append(u['url'])
    else:
        aid, cid, title, artist = await get_page_info(bili_url)
        if headers['Cookie'] != '':
            utoken = await get_utoken(aid, cid)
        # print(utoken)
        url = get_api_url(cid)
        if headers['Cookie'] != '':
            url = url + f'&utoken={utoken}'
        async with aiohttp.ClientSession() as session:
            async with session.request('get', url, headers=headers) as r:
                j = await r.json()
                # print(j)
                for u in j['durl']:
                    real_url.append(u['url'])

    print("title:" + title)
    for u in real_url:
        print(u)

asyncio.run(main())
