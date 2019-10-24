1. Follow the link that explains how to create a simple button

https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Add_a_button_to_the_toolbar

2. Publish the addon
  https://extensionworkshop.com/documentation/publish/package-your-extension/
  zip -r -FS ../kore.zip * --exclude *.git*

3. login at Access http://addons.mozilla.org
  user: coredev@coreblockch
  password: *********

4. Deploy the addon, after being reviewed it will get an uuid

5. Install the addon in a tor browser

6. Get the xpi from tor browser installation directory: <install>tor-browser_en-US/Browser/TorBrowser/Data/Browser/profile.default/extensions

7. In order to deploy the addon with the tor broser. 
   It is only necessary to create the directory: to tor-browser_en-US/Browser/distribution/extensions
   copy the xpi file to tor-browser_en-US/Browser/distribution/extensions

======================================================
Updating the Addon
1. Login to mozilla

2. Click on "Developer hub"

3. Find the Add on and Click in the "Approved"

4. "Upload a new version"