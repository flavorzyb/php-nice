<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('lnice')) {
	dl('lnice.' . PHP_SHLIB_SUFFIX);
}
$module = 'lnice';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$result = 0;
for ($i = 0; $i < 100000; $i++)
{
	$fp = fopen("a.txt", "wb+");
	fwrite($fp, "a");
	fclose($fp);
	$result .= $i;
}

$str = lnice_get_cpu_info();
var_dump($str);
var_dump(memory_get_usage());