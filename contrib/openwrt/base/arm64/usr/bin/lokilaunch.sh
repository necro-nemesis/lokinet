#!/bin/sh

case "$1" in

  start)
        service lokinet start
        ;;

  stop)
        service lokinet stop
        ;;

  exitup)
	
	if [ "$(uci get lokinet.@Address[0].Addr)" != "null" ]; then

	# Generate temp file in /etc/uci-defaults for defaults on reboot
		echo "		uci set dhcp.@dnsmasq[0].noresolv='0'
		uci set dhcp.@dnsmasq[0].localservice='0'
        	uci set dhcp.@dnsmasq[0].boguspriv='0'
        	uci commit dhcp
        	uci set lokinet.@Address[0].Addr='null'
        	uci set lokinet.@Key[0].Key='null'
	        uci commit lokinet" >> /etc/uci-defaults/xx_custom

		uci set dhcp.@dnsmasq[0].noresolv='1'
		uci set dhcp.@dnsmasq[0].localservice='1'
		uci delete dhcp.@dnsmasq[0].allserver='1'
		uci delete dhcp.@dnsmasq[0].boguspriv='0'
		uci commit dhcp
        	if [ "$(uci -q get lokinet.@Key[0].Key)" = "null" ]; then
			/usr/local/bin/lokinet-vpn --down			
                	/usr/local/bin/lokinet-vpn --up --exit "$(uci get lokinet.@Address[0].Addr)" &
        	else
			/usr/local/bin/lokinet-vpn --down
                	/usr/local/bin/lokinet-vpn --up --exit "$(uci get lokinet.@Address[0].Addr)" --token "$(uci get lokinet.@Key[0].Key)" &
	       	fi
		
		else
			
			/usr/local/bin/lokinet-vpn --down &
			uci set dhcp.@dnsmasq[0].noresolv='0'
			uci set dhcp.@dnsmasq[0].localservice='0'
			uci set dhcp.@dnsmasq[0].boguspriv='0'
        		uci commit dhcp
	fi
       ;;

  exitdown)
        /usr/local/bin/lokinet-vpn --down & 
	uci set dhcp.@dnsmasq[0].noresolv='0'
	uci set dhcp.@dnsmasq[0].localservice='0'
	uci set dhcp.@dnsmasq[0].boguspriv='0'
        uci commit dhcp
	uci set lokinet.@Address[0].Addr='null'
	uci set lokinet.@Key[0].Key='null'
	uci commit lokinet
        ;;

  *)
        echo "Usage: ""$1"" {start|stop|exitup|exitdown}"
        exit 1
        ;;
        esac
