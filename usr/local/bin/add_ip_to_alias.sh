#!/usr/local/bin/php -f
<?php
require_once("add_ip_to_alias.inc");
if (count($argv)<3 || count($argv)>4)
{
usage();
}
else
{
 add_ip_to_alias($argv[1],$argv[2],$argv[3]);
}

function usage()
{
 global $argv;
 echo "Usage:\n";
 echo "\t$argv[0] ALIAS_NAME IP_ADDRESS [LOG_COMMENT]\n";
}
?>