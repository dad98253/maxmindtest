# maxmindtest
A program to load maxmind csv files and verify them against the MaxMind GeoIP binary database.

The program also has the ability to filter the files based on country code and produce iptables, ipset, 
or ufw output appropriate for GeoIP filtering at a linux firewall. It also has an (untested) output
format for BSD pf tables.

The filter is conservative, in that it will block all IP addresses associated with the specified
country: The Maxmind database has three country fields associated with each IP address: "country",
"registered country", and "represented country". If any of these three are a match, the entry is
assumed to be a "match". Satellite and proxy flags are ignored.

The program takes up to six arguments. The first three are file paths, the fourth is the country
code to filter on, the fifth is the output format option, and the last is the interface to apply
the filter to. Each has a default. The default can also be invoked by specifying an "*" as the 
argument that you wish to use the default for.

The arguments and default values are:

1) The Maxmind GeoIp (binary) database file<br>
   default: "/var/lib/GeoIP/GeoLite2-Country.mmdb"
2) The Maxmind country location csv file<br>
   default: "GeoLite2-Country-Locations-en.csv"
3) The Maxmind country block csv file<br>
   default: "GeoLite2-Country-Blocks-IPv4.csv"
4) The two letter iso country code for the country to filter on<br>
   default: "CN" (China)
5) The format of the output file: 1 -> csv format similar the the input with iso code added, 2 -> iptables output, 3 -> ufw output, 4 -> BSD pf table output, 5 -> ipset output<br>
   default: csv format
6) The out facing interface on the firewall, or the setname (for ipset output)<br>
   default: "eth0" or "blacklist" (depending on output option)

The program needs to be linked against both the maxmind database library and libcsv:<br>

In order to get copies of the Maxmind database, you'll need a key. The key for the GeoLite
products is free. But, you'll need to go to their web site to get it:<br>
https://www.maxmind.com/en/geolite2/signup?lang=en <br>
Follow their instructions for setting up your key. Download the csv files that you want
from their web site. Finally, run the geoipupdate application to download their latest
(binary) GeoLite database. Note: I have had difficulty getting this to work on older
32 bit ubuntu systems. You may need a more recent distro. It works fine on 64 bit ubuntu 20.04.

sudo apt-get install libcsv-dev libmaxminddb-dev geoipupdate<br>
geoipupdate
git clone <this github repo><br>
cd maxmindtest<br>
./configure<br>
make<br>
bin/maxmindtest "*" "*" "*" "*" 2  > iptables.txt<br>

Have fun!<br>
-John

P.S.,
   I compared the output of this program with GeoIp block lists from other sources and they don't 
   appear to be in 100% agreement. When I spot check some of these results using dig, it's not
   clear how they get their data. I think that the best we can hope is that it is mostly correct.
   A "mostly correct" internet block is probably better that no block at all!
   
   Don't forget to take care to not lock youself out of your server. Allways whitelist yourself at
   the top of your tables!
   
   If the make insists that you install the auto tools, you could just compile the program without
   using make (it's only one file!):<br>
   gcc -o maxmindtest maxmindtest.c -lsv -lmaxminddb<br>
