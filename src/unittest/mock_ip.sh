#!/bin/sh

# This mock expects to be called as /sbin/ip --json addr show, /sbin/ip --json -4 route show or /sbin/ip --json -6 route show

if [ -f /tmp/invalid_parse_addr ] && [ $2 = "addr" ]; then
    cat << EOF
    {;]}
EOF
elif [ -f /tmp/invalid_parse_route4 ] && [ $3 = "route" ] && [ $2 = "-4" ]; then
    cat << EOF
    {"test":27,}
EOF
elif [ -f /tmp/invalid_parse_route6 ] && [ $3 = "route" ] && [ $2 = "-6" ]; then
    cat << EOF
    {"a","b"}
EOF
elif [ -f /tmp/invalid_type_addr ] && [ $2 = "addr" ]; then
    cat << EOF
    {}
EOF
elif [ -f /tmp/invalid_type_route4 ] && [ $3 = "route" ] && [ $2 = "-4" ]; then
    cat << EOF
    "invalid"
EOF
elif [ -f /tmp/invalid_type_route6 ] && [ $3 = "route" ] && [ $2 = "-6" ]; then
    cat << EOF
    23
EOF
elif [ $2 = "addr" ]; then
    cat << EOF
[\
{"ifindex":1,"ifname":"lo","flags":["LOOPBACK","UP","LOWER_UP"],\
"mtu":65536,"qdisc":"noqueue","operstate":"UNKNOWN","group":"default","txqlen":1000,"link_type":"loopback",\
"address":"00:00:00:00:00:00","broadcast":"00:00:00:00:00:00","addr_info":[\
{"family":"inet","local":"127.0.0.1","prefixlen":8,"scope":"host","label":"lo","valid_life_time":4294967295,"preferred_life_time":4294967295},\
{"family":"inet6","local":"::1","prefixlen":128,"scope":"host","valid_life_time":4294967295,"preferred_life_time":4294967295}]},\
{"ifindex":2,"ifname":"eth0","flags":["BROADCAST","MULTICAST","UP","LOWER_UP"],\
"mtu":1508,"qdisc":"fq_codel","operstate":"UNKNOWN","group":"default","txqlen":1000,"link_type":"ether",\
"address":"aa:bb:cc:dd:ee:eff","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[\
{"family":"inet6","local":"fe80::1234:5678:9abc:def0","prefixlen":64,"scope":"link","valid_life_time":4294967295,"preferred_life_time":4294967295}]},\
{"ifindex":3,"link":"eth0","ifname":"wan","flags":["NO-CARRIER","BROADCAST","MULTICAST","UP"],\
"mtu":1500,"qdisc":"noqueue","master":"br-wan","operstate":"LOWERLAYERDOWN","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:12:34:56:78:9a","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[]},\
{"ifindex":7,"link":"eth0","ifname":"lan2","flags":["BROADCAST","MULTICAST","UP","LOWER_UP"],\
"mtu":1500,"qdisc":"noqueue","master":"br-lan","operstate":"UP","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:17:9d:f3:5b:df","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[]},\
{"ifindex":8,"link":null,"ifname":"ip6tnl0","flags":["NOARP"],\
"mtu":1452,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"tunnel6",\
"address":"::","broadcast":"::","addr_info":[]},\
{"ifindex":9,"link":null,"ifname":"sit0","flags":["NOARP"],\
"mtu":1480,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"sit",\
"address":"0.0.0.0","broadcast":"0.0.0.0","addr_info":[]},\
{"ifindex":10,"link":null,"ifname":"gre0","flags":["NOARP"],\
"mtu":1476,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"gre",\
"address":"0.0.0.0","broadcast":"0.0.0.0","addr_info":[]},\
{"ifindex":11,"link":null,"ifname":"gretap0","flags":["BROADCAST","MULTICAST"],\
"mtu":1462,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:00:00:00:00:00","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[]},\
{"ifindex":12,"link":null,"ifname":"erspan0","flags":["BROADCAST","MULTICAST"],\
"mtu":1450,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:00:00:00:00:00","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[]},\
{"ifindex":13,"ifname":"teql0","flags":["NOARP"],\
"mtu":1500,"qdisc":"noop","operstate":"DOWN","group":"default","txqlen":100,"link_type":"void","addr_info":[]},\
{"ifindex":14,"ifname":"br-lan","flags":["BROADCAST","MULTICAST","UP","LOWER_UP"],\
"mtu":1500,"qdisc":"noqueue","operstate":"UP","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:19:28:37:46:55","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[\
{"family":"inet","local":"192.168.1.77","prefixlen":24,"broadcast":"192.168.1.255","scope":"global","label":"br-lan","valid_life_time":4294967295,"preferred_life_time":4294967295},\
{"family":"inet6","local":"fe80::2234:5678:9abc:def0","prefixlen":64,"scope":"link","valid_life_time":4294967295,"preferred_life_time":4294967295}]},\
{"ifindex":15,"ifname":"br-wan","flags":["NO-CARRIER","BROADCAST","MULTICAST","UP"],\
"mtu":1500,"qdisc":"noqueue","operstate":"DOWN","group":"default","txqlen":1000,"link_type":"ether",\
"address":"00:1f:2e:3c:4b:5a","broadcast":"ff:ff:ff:ff:ff:ff","addr_info":[]},\
{"ifindex":18,"ifname":"tun0","flags":["POINTOPOINT","MULTICAST","NOARP","UP","LOWER_UP"],\
"mtu":1500,"qdisc":"fq_codel","operstate":"UNKNOWN","group":"default","txqlen":500,"link_type":"none","addr_info":[\
{"family":"inet","local":"17.16.15.14","prefixlen":24,"scope":"global","label":"tun0","valid_life_time":4294967295,"preferred_life_time":4294967295},\
{"family":"inet6","local":"fe80::3344:5566:7788:99aa","prefixlen":64,"scope":"global","stable-privacy":true,"valid_life_time":4294967295,"preferred_life_time":4294967295}]},\
"invalidobject",\
{"objectwithoutifname":true},\
{"ifname":27,"objectwithifnamenotstring":true},\
{"ifname":"withoutflags"},\
{"ifname":"withinvalidflags","flags":27},\
{"ifname":"withinvalidflags","flags":[1,2,"three"]},\
{"ifname":"noaddrinfo","flags":[]},\
{"ifname":"invalidaddrinfo","flags":[],"addr_info":"invalidtype"},\
{"ifname":"invalidaddrinfo","flags":[],"addr_info":[1,2,"three"]},\
{"ifname":"noscope","flags":[],"addr_info":[{}]},\
{"ifname":"invalidscope","flags":[],"addr_info":[{"scope":23}]},\
{"ifname":"missingfamily","flags":[],"addr_info":[{"scope":"global"}]},\
{"ifname":"invalidfamily","flags":[],"addr_info":[{"scope":"global","family":23}]},\
{"ifname":"missinglocal","flags":[],"addr_info":[{"scope":"global","family":"some"}]},\
{"ifname":"invalidlocal","flags":[],"addr_info":[{"scope":"global","family":"some","local":23}]},\
{"ifname":"missingprefix","flags":[],"addr_info":[{"scope":"global","family":"some","local":"some"}]},\
{"ifname":"invalidprefix","flags":[],"addr_info":[{"scope":"global","family":"some","local":"some","prefixlen":"string"}]},\
{"ifname":"invalidprefix","flags":[],"addr_info":[{"scope":"global","family":"some","local":"some","prefixlen":23}]}\
]
EOF
elif [ $3 = "route" ] && [ $2 = "-4" ]; then
    cat << EOF
[\
"invalid",\
{},\
{"dev":27},\
{"dev":"tun0"},\
{"dev":"tun0","gateway":"some","dst":23},\
{"dst":"default","gateway":"192.168.1.1","dev":"br-lan","protocol":"static","metric":2,"flags":[],"metrics":[{"mtu":1500}]},\
{"dst":"10.0.0.0/16","gateway":"17.16.15.14","dev":"tun0","flags":[]},\
{"dst":"18.19.20.0/24","dev":"tun0","protocol":"kernel","scope":"link","prefsrc":"18.19.20.22","flags":[]},\
{"dst":"192.168.1.0/24","dev":"br-lan","protocol":"kernel","scope":"link","prefsrc":"192.168.1.77","flags":[]}\
]
EOF
elif [ $3 = "route" ] && [ $2 = "-6" ]; then
    cat << EOF
[\
"invalid",\
{},\
{"dev":27},\
{"dev":"tun0"},\
{"dev":"tun0","gateway":"some","dst":23},\
{"dst":"fe80::/64","dev":"eth0","protocol":"kernel","metric":256,"flags":[],"pref":"medium"},\
{"dst":"fe80::/64","dev":"br-lan","protocol":"kernel","metric":256,"flags":[],"pref":"medium"},\
{"dst":"fe80::/64","dev":"tun0","protocol":"kernel","metric":256,"flags":[],"pref":"medium"},\
{"dst":"default","gateway":"fe80::1111:2222:3333:4444","dev":"tun0","protocol":"kernel","metric":256,"flags":[],"pref":"medium"}\
]
EOF
fi

exit 0