m = Map("lokinet", translate("Exit Node Settings"), translate("Fill in the exit information of your provider"))
d = m:section(TypedSection, "Address")
a = d:option(Value, "Addr", "Lokinet Address:") a.optional=false; a.rmempty = false;
v = m:section(TypedSection, "Key") 
w = v:option(Value, "Key", "(Optional) Lokinet Key:")
d.anonymous = true
v.anonymous = true
r = m:section(NamedSection, "Enabled")
x = r:option(Value, "Enabled", "1");

        btnclear = v:option(Button, "Clear", translate("Deactivate Exit"))
	btnon = v:option(Button, "Enabled", translate("Lokinet Start"))
	btnoff = v:option(Button, "Disabled", translate("Lokinet Stop"))

function btnclear.write()
        luci.sys.exec("uci set lokinet.@Address[0].Addr=null")
	luci.sys.exec("uci set lokinet.@Key[0].Key=null")
	luci.sys.exec("uci commit lokinet")
	luci.sys.exec("/usr/bin/lokilaunch.sh exitdown")
        end

function btnon.write()
	luci.sys.exec("/etc/init.d/lokinet start")
	screen = v:option(DummyValue, "option", "Status", "Enabled")
	end

function btnoff.write()
	luci.sys.exec("/etc/init.d/lokinet stop")
	screen = v:option(DummyValue, "option", "Status", "Disabled")
	end

m.apply_on_parse = true
m.on_after_apply = function(self)
	luci.sys.exec("/usr/bin/lokilaunch.sh exitup")
    	end

return m
