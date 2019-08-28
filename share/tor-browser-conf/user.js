// User configuration for tor-browser
// these configuration were retrieved from https://www.ghacks.net/overview-firefox-aboutconfig-security-privacy-preferences/



// load browser's home page
// This defines how Firefox will start up.
// 0: load a blank page (about:blank)
// 1: load the browser's homepage. (default)
// 2: load the last visited page
// 3: resume the previous browser session.
user_pref("browser.startup.page", 1);

// Security Level
// 1: Safest
// 2: Safer
// 3: Standard
user_pref("extensions.torbutton.security_slider", 2);

// Determines whether websites are allowed to access clipboard contents 
// (check out: Block websites from reading or modifying Clipboard contents in Firefox for additional information).
user_pref("dom.event.clipboardevents.enabled", false);

// Determines if location aware browsing is enabled.
user_pref("geo.enabled", false);

// Defines when to set the referrer (the page a visit originated from).
user_pref("network.http.referer.XOriginPolicy", 0);

// Defines which sets of data get cleared when Firefox shuts down. 
// A value of true means the data set is cleared on exit, false that it is kept.
user_pref("privacy.clearOnShutdown.*", true);

// Whether the browsing history is automatically cleared on shutdown.
user_pref("privacy.sanitize.sanitizeOnShutdown", true);

// Defines if OCSP Stapling is enabled in Firefox which determines how certificate information are retrieved 
// (check Firefox 25 gets OCSP Stapling which improves privacy for detailed information).
user_pref("security.OCSP.enable", 1);
user_pref("security.tls.version.min", 3);
user_pref("security.tls.version.max", 3);

// This preference determines whether WebRTC is enabled in Firefox. 
// WebRTC is used for telephony and video chat functionality but leaks local and remote IP addresses as well. 
// May also be used in browser fingerprinting.
user_pref("media.peerconnection.enabled", false);

// Provides web applications with information about video playback statistics such as the framerate.
user_pref("media.video_stats.enabled", false);

// 0: All cookies are allowed.
// 1: Only cookies from the first-party server are allowed.
// 2: Block all cookies.
// 3: Third-party cookies are only allowed if cookies from the site are already stored by Firefox. (default)
user_pref("network.cookie.cookieBehavior", 2);

// 0: The originated server sets the cookie lifetime. (default)
// 1: Firefox prompts the user (unless network.cookie.alwaysAcceptSessionCookies is set to true).
// 2: Cookie expires at the end of the session.
// 3: The cookie lasts for the days specified in network.cookie.lifetime.days.
user_pref("network.cookie.lifetimePolicy", 2);

// 0: Never send the Referer header or set document.referrer.
// 1: Send it after clicking on links.
// 2. Send if after clicking on links or loading an image (default).
user_pref("network.http.sendRefererHeader", 0);

// Defines whether Firefox caches http requests.
user_pref("network.http.use-cache", true);

//Scans the Windows Registry key for plugin references. If found, adds them to Firefox.
user_pref("plugin.scan.plid.all", false);
