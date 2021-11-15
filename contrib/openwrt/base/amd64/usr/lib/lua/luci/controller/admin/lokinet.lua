module("luci.controller.admin.lokinet", package.seeall)  --notice that new_tab is the name of the file new_tab.lua
 function index()
     entry({"admin", "lokinet"}, firstchild(), "Lokinet", 60).dependent=false  --this adds the top level tab and defaults to the first sub-tab (tab_from_cbi), also it is set to position 30
     entry({"admin", "lokinet", "tab_from_lokinet"}, cbi("lokinet-module/lokinet_tab"), "Exit Settings", 1)  --this adds the first sub-tab that is located in <luci-path>/luci-myapplication/model/cbi/myapp-mymodule and the file is called cbi_tab.lua, also set to first position
end
