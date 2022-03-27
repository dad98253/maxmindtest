# maxmindtest
A program to load maxmind csv files and verify them against the MaxMind GeoIP binary database.

The program also has the ability to filter the files based on country code and produce iptables
or ufw output appropriate for GeoIP filtering at a linux firewall.

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
4) The iso country code for the country to filter on<br>
   default: "CN" (China)
5) The format of the output file: 1 -> csv format similar the the input with iso code added, 2 -> iptables output, 3 -> ufw output, 4 -> BSD pf table output<br>
   default: csv format
6) The out facing interface on the firewall<br>
   default: "eth0"

The program needs to be linked against both the maxmind database library and libcsv:<br>
sudo apt-get install libcsv-dev libmaxminddb-dev<br>
git clone ...<br>
cd maxmindtest<br>
./configure<br>
make<br>
bin/maxmindtest "*" "*" "*" "*" 2  > iptables.txt<br>

Have fun!<br>
-John
