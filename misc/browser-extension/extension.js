function checkUrl(u) {

    if (u.includes('bilibili.com/s/video')) {
        u = u.replace('bilibili.com/s/video', 'bilibili.com/video');
    }
    var url = new URL(u);
    url.protocol = 'dmlive:';
    if (u.includes('bilibili.com')) {
        url = url.protocol + url.hostname + url.pathname + (url.searchParams.get('p') ? '?p=' + url.searchParams.get('p') : '');
    } else if (u.includes('youtube.com/watch')) {
        url = url.protocol + url.hostname + url.pathname + (url.searchParams.get('v') ? '?v=' + url.searchParams.get('v') : '');
    } else {
        url = url.protocol + url.hostname + url.pathname;
    }
    if (u.includes('bilibili.com/video') || u.includes('bilibili.com/bangumi/play') || u.includes('live.bilibili.com/') || u.includes('twitch.tv/') || u.includes('douyu.com/') || u.includes('huya.com/') || u.includes('youtube.com/watch') || u.includes('youtube.com/channel/')) {
        return url;
    } else {
        return "";
    }
}

chrome.contextMenus.removeAll(()=>{
    chrome.contextMenus.create({
        id: "open_with_qlp",
        title: "Open With QLivePlayer",
        contexts: ["selection", "link", "page"],
    });
}
);

chrome.contextMenus.onClicked.addListener((info,tab)=>{
    switch (info.menuItemId) {
    case "open_with_qlp":
        console.log(info);
        var url = "";
        if ("selectionText"in info) {
            url = checkUrl(info.selectionText);
        } else if ("linkUrl"in info) {
            url = checkUrl(info.linkUrl);
        } else if ("pageUrl"in info) {
            url = checkUrl(info.pageUrl);
        }
        if (url !== "") {
            console.log(url);
            chrome.tabs.create({
                url: url,
                active: true
            });
        }
        break;
    }
}
);
