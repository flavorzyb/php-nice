<?php
include "Nice.php";
if(!extension_loaded('lnice')) {
	dl('lnice.' . PHP_SHLIB_SUFFIX);
}

$result = 0;
for ($i = 0; $i < 100000; $i++)
{
	$fp = fopen("a.txt", "wb+");
	fwrite($fp, "a");
	fclose($fp);
	$result .= $i;
}
$nice = new Helper_Nice();
$filename = sprintf("%s/nice_%s.log", dirname(__FILE__), date('Y_m_d'));
if ($nice->loadTaskNice() && ($nice->getNice() > 0.1))
{
	if (file_exists($filename) && (filesize($filename) > (800*1024*1024)))
	{
		@rename($filename, $filename.date('h_i_s'));
	}
	
	$fp = @fopen($filename, "ab+");
	if ($fp)
	{
		$query_url  = isset($_SERVER["REQUEST_URI"])        ? $_SERVER["REQUEST_URI"] : '';
		$refer      = isset($_SERVER['HTTP_REFERER'])       ? $_SERVER['HTTP_REFERER'] : '';
		$method		= isset($_SERVER["REQUEST_METHOD"])       ? $_SERVER["REQUEST_METHOD"] : '';
		
		$str = sprintf(	"time:%s, pid:%d, cpu:%d, nice:%.1f, u_nice:%.1f, s_nice:%.1f, ".
						"idle:%.1f, iowait:%.1f, irq:%.1f, softirq: %.1f, " .
						"totalTime:%d, vss:%.1fMB, rss:%.1fMB, rlim:%.1fMB, ".
						"uri:%s, refer:%s, method:%s\n",
						date('Y-m-d H:i:s'),
						$nice->getPid(),
						$nice->getCPU(),
						$nice->getNice(),
						$nice->getUserNice(),
						$nice->getSysNice(),
						$nice->getIDLE(),
						$nice->getIOWAIT(),
						$nice->getIRQ(),
						$nice->getSoftIRQ(),
						$nice->getTotalTime(),
						$nice->getVSS() / 1024,
						$nice->getRSS() / 1024,
						$nice->getRLIM() / 1024,
						$query_url,
						$refer,
						$method
						);
		fwrite($fp, $str);
		fclose($fp);
	}
}
	