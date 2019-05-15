
// adding tor as a sub-module
https://chrisjean.com/git-submodules-adding-using-removing-and-updating/
1) Add tor as submodule
   git submodule add https://git.torproject.org/tor.git  tor

2) Changing tor version
  cd tor
  git checkout tor-0.3.4.9
  cd ..
  git add tor
  git commit -m "tor version 0.3.4.9"
  git push

   
3) Updating the tor branch
git submodule init
git submodule update   

4) Checkin tor version
  git log will tell the commit number!!!
