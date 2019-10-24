
function onRemovedCookies() {
  console.log("Removed all cookies");
}

function onRemovedHistory() {
  console.log("Removed History");
}

function onError(error) {
  console.log(`Error removing cookie: ${error}`);
}

document.addEventListener("click", function(e) {
  if (e.target.classList.contains("clear_cookies")) {
    browser.browsingData.removeCookies({}).then(onRemovedCookies, onError);     
  } else if (e.target.classList.contains("clear_history")) {
    browser.browsingData.removeHistory({}).then(onRemovedHistory, onError);      
  }
});