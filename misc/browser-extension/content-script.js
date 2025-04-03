chrome.runtime.onMessage.addListener((message) => {
  if (message.action === "dmlive-open-link") {
    window.open(message.url, '_self');
  }
});
