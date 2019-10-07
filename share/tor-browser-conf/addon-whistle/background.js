function openPage() {
  browser.tabs.create({
    url: "https://wikileaks.org/#submit_help_tips"
  });
}

browser.browserAction.onClicked.addListener(openPage);
