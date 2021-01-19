// ==UserScript==
// @name         Click to QLivePlayer
// @version      0.2.0
// @description  Open QLivePlayer instead when click on video link in BiliBili
// @author       Chinory
// @homepage     https://github.com/Chinory/tampermonkey-scripts/wiki/click-to-qliveplayer.js
// @match        *://*.bilibili.com/*
// @grant        unsafeWindow

// ==/UserScript==
"use strict";

var open = unsafeWindow.open;

function rewrite(l) {
  var s = l.pathname;
  switch (l.hostname) {
    case "www.bilibili.com":
      if (s.slice(0, 7) === "/video/" || s.slice(0, 14) === "/bangumi/play/") {
        var p = l.searchParams.get("p");
        if (p) s = s + "?p=" + p;
        break;
      } else return;
    case "live.bilibili.com":
      if (/^\/\d+$/.test(s)) break;
      else return;
    default:
      return;
  }
  return "qliveplayer://" + l.host + s;
}

unsafeWindow.addEventListener(
  "click",
  function (e) {
    var a, l, u;
    (a = e.target.closest("a")) &&
      (l = new URL(a.href, this.location.href)) &&
      (u = rewrite(l)) &&
      open.call(this, u, "_self") &&
      e.preventDefault();
  },
  true
);

unsafeWindow.open = function (a) {
  var l = new URL(a, this.location.href);
  var u = rewrite(l);
  if (u) return open.call(this, u, "_self");
  else return open.apply(this, arguments);
};
