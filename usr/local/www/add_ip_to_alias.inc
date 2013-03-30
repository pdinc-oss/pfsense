<?php

include_once("auth.inc");
include_once("priv.inc");

require_once("functions.inc");
require_once("filter.inc");
require_once("shaper.inc");

function add_ip_to_alias($name, $ip, $comment=NULL)
{
	global $config;
	
	if (!is_array($config['aliases']['alias'])) $config['aliases']['alias'] = array();
	$a_aliases = &$config['aliases']['alias'];
	
	$alias=NULL;
	$id=NULL;
	
	foreach ($a_aliases as $index => $lalias)
	{
		if ($lalias['name']===$name)
		{
			$alias=$lalias;
			$id=$index;
			break;
		}
	}
	if ($alias==NULL)
	{
		logerror("Alias not found!");
		return false;
	}
	
	if (is_alias($ip)) 
	{
		if (!alias_same_type($ip, $alias['type']))
		{		
			logerror("Alias type mismatch!");
			return false;
		}
	}
	else if (!is_ipaddr($ip) && !is_hostname($ip) && !is_iprange($ip))
	{
		logerror("invalid ip address!");
		return false;
	}
	
	$address_delim=" ";
	$detail_delim="||";
	
	$addresses=explode($address_delim, $alias['address']);
	$addresses[]=$ip;
	$alias['address'] = implode(" ", $addresses);
	

	$details=explode($detail_delim,$alias['detail']);
	$details[] =($comment <> "")? str_replace($detail_delim,"",$comment):sprintf(gettext("Entry added %s"), date('r'));
	$alias['detail'] = implode($detail_delim, $details);
	

	$a_aliases[$id]=$alias;
	
	mark_subsystem_dirty('aliases');
	
	write_config();
	
	$retval = 0;
	
	/* reload all components that use aliases */
	$retval = filter_configure_sync();
	
	if(stristr($retval, "error") <> true)
	{
		logdebug("filter_configure()=$retval");
		// no gui here...
		//$savemsg = get_std_save_message($retval);
	}
	else
	{
		$savemsg = $retval;
		logerror($savemsg);
	}
	if ($retval == 0)
	{
		clear_subsystem_dirty('aliases');
	}
	else
	{
		logerror("how did we get here!");
	}
	
	
	
	do 
	{
		$states=array();
		exec("/sbin/pfctl -s state | grep " . escapeshellarg(htmlspecialchars($ip)), $states);
		//var_dump($states);
		foreach($states as $line) 
		{
	
			$line_split = preg_split("/\s+/", $line);
			$type  = array_shift($line_split);
			$proto = array_shift($line_split);
			$state = array_pop($line_split);
			$info  = implode(" ", $line_split);
	
			/* break up info and extract $srcip and $dstip */
			$ends = preg_split("/\<?-\>?/", $info);
			$parts = split(":", $ends[0]);
			$srcip = trim($parts[0]);
			$parts = split(":", $ends[count($ends) - 1]);
			$dstip = trim($parts[0]);

			if (is_ipaddr($srcip) and is_ipaddr($dstip)) 
			{
				$cmd="/sbin/pfctl -k " . escapeshellarg($srcip) . " -k " . escapeshellarg($dstip);
				logdebug($cmd);
				$retval = mwexec($cmd);
			} else {
				logerror("invalid state removal [$srcip,$dstip]");
			}			
		}
	}
	while (count($states) > 0);
}

function logerror($msg)
{
	echo "$msg\n";
}
function logdebug($msg)
{
	//echo "debug:$msg\n";
}
?>
