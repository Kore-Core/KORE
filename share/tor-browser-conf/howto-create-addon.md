1. Follow the link that explains how to create a simple button

https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Add_a_button_to_the_toolbar

2. login at Access http://addons.mozilla.org
  user: coredev@coreblockch
  password: *********

3. Deploy the addon, after being reviewed it will get an uuid

4. Install the addon in a tor browser

5. Get the xpi from tor browser installation directory: <install>tor-browser_en-US/Browser/TorBrowser/Data/Browser/profile.default/extensions

6. In order to deploy the addon with the tor broser. 
   It is only necessary to create the directory: to tor-browser_en-US/Browser/distribution/extensions
   copy the xpi file to tor-browser_en-US/Browser/distribution/extensions