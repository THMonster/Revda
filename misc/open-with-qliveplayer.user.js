// ==UserScript==
// @name         Open With QLivePlayer
// @version      0.1.0
// @description  Open with qliveplayer!
// @author       IsoaSFlus
// @match        *://*.bilibili.com/*
// @match        *://bilibili.com/*
// @grant        unsafeWindow


// ==/UserScript==

(function() {
    'use strict';
    var mousemove_handler = function(e) {
        var url = window.location.href;
        var el = e.target;
        //console.log(el);
        while (1) {
            if (el.querySelector('[href]')) {
                if (check_url(el.querySelector('[href]').href)) {
                    el = el.querySelector('[href]')
                    break;
                }
            } else if ('href' in el) {
                if (check_url(el.href)) {
                    break;
                }
            }
            el = el.parentElement;
            if (el === null) {
                break;
            }
        }
        if (el !== null) {
            url = el.href;
        }
        url = new URL(url);
        url = url.hostname + url.pathname + (url.searchParams.get('p') ? '?p=' + url.searchParams.get('p') : '');
        console.log(url);
        if (!check_url(url)) {
            return;
        }
        window.open('qliveplayer://' + url, '_self');
        e.preventDefault();
    };
    var check_url = function(u) {
        if (u.includes('bilibili.com/video') || u.includes('bilibili.com/bangumi/play')) {
            if (u.includes('/av')) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    };

    document.addEventListener('keyup', function(e) {
        // pressed alt+q
        if (e.keyCode == 81 && !e.shiftKey && !e.ctrlKey && e.altKey && !e.metaKey) {
            document.addEventListener('mousemove', mousemove_handler, {once: true});
        }
    }, false);



})();
