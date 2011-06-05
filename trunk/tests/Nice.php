<?php
/**
 * 获取PHP进程的CPU和内存的使用情况
 *
 * @filesource Nice.php
 * @copyright   (c) 2009
 * @author  Yanbin Zhu<haker-haker@163.com>
 * @package Helper
 * @subpackage Helper_Nice
 */
class Helper_Nice
{
	/**
	 * PHP 模块名
	 *
	 * @var string
	 */
	const EXTENSION_NAME = 'lnice';
	
	/**
	 * 最小响应时间
	 * 0.2秒
	 *
	 * @var int
	 */
	//const TOTAL_TIME_MIN = 20;
	const TOTAL_TIME_MIN = 100;
	
	/**
	 *
	 * 正在运行的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_RUNNING 	= 'R';
	/**
	 *
	 * 已经休眠的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_SLEEPING 	= 'S';
	/**
	 *
	 * 已经休眠到硬盘中的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_DISK_SLEEP	= 'D';
	/**
	 *
	 * 已经停止的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_STOPPED	= 'T';
	/**
	 *
	 * 僵死的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_ZOMBIE		= 'Z';
	/**
	 *
	 * 已经死掉的进程
	 *
	 * @var string
	 */
	CONST TASK_STATE_DEAD		= 'X';
	
	/**
	 *
	 * 进程ID号
	 * @var int
	 */
	protected $_pid			= NULL;
	
	/**
	 * 运行在哪个CPU上
	 *
	 *
	 * @var int
	 */
	protected $_cpu			= 0;
	
	/**
	 * 进程运行状态
	 *
	 * @var string
	 */
	protected $_state		= self::TASK_STATE_RUNNING;
	
	/**
	 * CPU使用率
	 * @var float
	 */
	protected $_nice		= 0;
	
	/**
	 * 用户对CPU使用率
	 * @var float
	 */
	protected $_userNice	= 0;
	
	/**
	 * 系统对CPU使用率
	 * @var float
	 */
	protected $_sysNice		= 0;
	
	/**
	 * 除硬盘IO等待时间以外其它等待时间 比率
	 *
	 * @var float
	 */
	protected $_idle		= 0;
	
	/**
	 * IO等待时间 比率
	 * @var float
	 */
	protected $_iowait		= 0;
	
	/**
	 * 硬中断时间 比率
	 *
	 * @var float
	 */
	protected $_irq			= 0;
	
	/**
	 * 软中断时间 比率
	 *
	 * @var float
	 */
	protected $_softirq		= 0;
	
	/**
	 * 总共的消耗时间
	 * 单位：jiffies (1 jiffies = 10 ms)
	 *
	 * @var int
	 */
	protected $_totalTime	= 0;
	
	/**
	 * 该任务的虚拟地址空间大小
	 * 单位: KB
	 * @var int
	 */
	protected $_vss			= 0;
	
	/**
	 * 该任务当前驻留物理地址空间的大小
	 * 单位: KB
	 * @var int
	 */
	protected $_rss			= 0;
	
	/**
	 * 该任务能驻留物理地址空间的最大值
	 * 单位: KB
	 * @var int
	 */
	protected $_rlim		= 0;

	/**
	 * 构造函数
	 *
	 * @return void
	 */
	public function __construct()
	{
		$this->setPid(getmypid());
		return $this;
	}
	
	/**
	 * 设置PID
	 *
	 * @param int $pid
	 * @return Helper_Nice
	 */
	protected function setPid($pid)
	{
		$this->_pid = intval($pid);
		return $this;
	}
	
	/**
	 * 获取PID
	 *
	 * @return intNice
	 */
	public function getPid()
	{
		return $this->_pid;
	}

	/**
	 * 任务运行状态
	 *
	 * @param string $state
	 * @return Helper_Nice
	 */
	protected function setState($state)
	{
		$state = trim($state);
		switch ($state)
		{
			case self::TASK_STATE_RUNNING:
				break;
			case self::TASK_STATE_SLEEPING:
				break;
			case self::TASK_STATE_DISK_SLEEP:
				break;
			case self::TASK_STATE_STOPPED:
				break;
			case self::TASK_STATE_ZOMBIE:
				break;
			case self::TASK_STATE_DEAD:
				break;
			default:
				$state = self::TASK_STATE_RUNNING;
		}
		
		$this->_state = $state;
		
		return $this;
	}

	/**
	 * pid
	 * 获取任务运行状态
	 *
	 * @return string
	 */
	public function getState()
	{
		return $this->_state;
	}

	/**
	 * 设置该任务所在CPU
	 *
	 * @param int $cpu
	 * @return Helper_Nice
	 */
	protected function setCPU($cpu)
	{
		$this->_cpu = intval($cpu);
		return $this;
	}
	
	/**
	 * 获取该任务所在CPU
	 *
	 * @return int
	 */
	public function getCPU()
	{
		return $this->_cpu;
	}
	
	/**
	 * 设置任务CPU使用率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setNice($nice)
	{
		$this->_nice = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取任务CPU使用率情况
	 *
	 * @return float
	 */
	public function getNice()
	{
		return $this->_nice;
	}
	
	/**
	 * 设置该任务用户操作对CPU使用率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setUserNice($nice)
	{
		$this->_userNice = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取任务用户操作对CPU使用率情况
	 *
	 * @return float
	 */
	public function getUserNice()
	{
		return $this->_userNice;
	}
	
	/**
	 * 设置任务系统操作对CPU使用率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setSysNice($nice)
	{
		$this->_sysNice = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取任务系统操作对CPU使用率情况
	 *
	 * @return float
	 */
	public function getSysNice()
	{
		return $this->_sysNice;
	}
	
	/**
	 * 设置除硬盘IO等待时间以外其它等待时间 比率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setIDLE($nice)
	{
		$this->_idle = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取除硬盘IO等待时间以外其它等待时间 比率
	 *
	 * @return float
	 */
	public function getIDLE()
	{
		return $this->_idle;
	}
	
	/**
	 * 设置IO等待时间 比率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setIOWAIT($nice)
	{
		$this->_iowait = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取IO等待时间 比率
	 *
	 * @return float
	 */
	public function getIOWAIT()
	{
		return $this->_iowait;
	}
	
	/**
	 * 设置硬中断时间 比率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setIRQ($nice)
	{
		$this->_irq = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取硬中断时间 比率
	 *
	 * @return float
	 */
	public function getIRQ()
	{
		return $this->_irq;
	}
	
	/**
	 * 设置软中断时间 比率
	 *
	 * @param float $nice
	 * @return Helper_Nice
	 */
	protected function setSoftIRQ($nice)
	{
		$this->_softirq = floatval($nice);
		return $this;
	}
	
	/**
	 * 获取软中断时间 比率
	 *
	 * @return float
	 */
	public function getSoftIRQ()
	{
		return $this->_softirq;
	}
	
	/**
	 * 设置任务总共的消耗时间
	 *
	 * 单位：jiffies (1 jiffies = 10 ms)
	 *
	 * @param int $jiffiesTime
	 * @return Helper_Nice
	 */
	protected function setTotalTime($jiffiesTime)
	{
		$this->_totalTime = intval($jiffiesTime);
		return $this;
	}
	
	/**
	 * 获取任务总共的消耗时间
	 *
	 * 单位：jiffies (1 jiffies = 10 ms)
	 *
	 * @return int
	 */
	public function getTotalTime()
	{
		return $this->_totalTime;
	}
	
	/**
	 * 设置任务CPU使用率
	 *
	 * @param int $vss
	 * @return Helper_Nice
	 */
	protected function setVSS($vss)
	{
		$this->_vss = intval($vss);
		return $this;
	}
	
	/**
	 * 获取任务CPU使用率情况
	 *
	 * @return int
	 */
	public function getVSS()
	{
		return $this->_vss;
	}
	/**
	 * 设置任务CPU使用率
	 *
	 * @param int $rss
	 * @return Helper_Nice
	 */
	protected function setRSS($rss)
	{
		$this->_rss = intval($rss);
		return $this;
	}
	
	/**
	 * 获取任务CPU使用率情况
	 *
	 * @return int
	 */
	public function getRSS()
	{
		return $this->_rss;
	}
	/**
	 * 设置任务CPU使用率
	 *
	 * @param int $rlim
	 * @return Helper_Nice
	 */
	protected function setRLIM($rlim)
	{
		$this->_rlim = intval($rlim);
		return $this;
	}
	
	/**
	 * 获取任务CPU使用率情况
	 *
	 * @return int
	 */
	public function getRLIM()
	{
		return $this->_rlim;
	}
	
	
	/**
	 * 获取该PHP进程的系统资源使用情况
	 *
	 * @return boolean
	 */
	public function loadTaskNice()
	{
		if (!extension_loaded(self::EXTENSION_NAME))
		{
			return false;
		}
		
		$result = lnice_get_cpu_info();
		
		if (false == $result)
		{
			return false;
		}
		
		$totalTime = (isset($result['total_time']) ? intval($result['total_time']) : 0);
		
		if ($totalTime < self::TOTAL_TIME_MIN)
		{
			return false;
		}
		
		$pid 		= (isset($result['pid']) 		? $result['pid'] 		: 0);
	    $cpu 		= (isset($result['cpu']) 		? $result['cpu'] 		: 0);
	    $state 		= (isset($result['state']) 		? $result['state'] 		: 0);
	    $nice 		= (isset($result['nice']) 		? $result['nice'] 		: 0);
	    $u_nice 	= (isset($result['u_nice']) 	? $result['u_nice'] 	: 0);
	    $s_nice 	= (isset($result['s_nice']) 	? $result['s_nice'] 	: 0);
	    $idle 		= (isset($result['idle']) 		? $result['idle'] 		: 0);
	    $iowait 	= (isset($result['iowait']) 	? $result['iowait'] 	: 0);
	    $irq 		= (isset($result['irq']) 		? $result['irq'] 		: 0);
	    $softirq 	= (isset($result['softirq'])	? $result['softirq']	: 0);
	    $vss 		= (isset($result['vss']) 		? $result['vss'] 		: 0);
	    $rss 		= (isset($result['rss']) 		? $result['rss'] 		: 0);
	    $rlim 		= (isset($result['rlim']) 		? $result['rlim'] 		: 0);
		
		$this	->setPid($pid)
				->setCPU($cpu)
				->setState($state)
				->setNice($nice)
				->setUserNice($u_nice)
				->setSysNice($s_nice)
				->setIDLE($idle)
				->setIOWAIT($iowait)
				->setIRQ($irq)
				->setSoftIRQ($softirq)
				->setTotalTime($totalTime)
				->setVSS($vss)
				->setRLIM($rlim);

		return true;
	}
	
	/**
	 * to array
	 *
	 * @return array
	 */
	public function toArray()
	{
		$data = array(
					'pid' 		=> $this->getPid(),
					'cpu' 		=> $this->getCPU(),
					'state' 	=> $this->getState(),
					'nice' 		=> $this->getNice(),
					'userNice' 	=> $this->getUserNice(),
					'sysNice' 	=> $this->getSysNice(),
					'idle' 		=> $this->getIDLE(),
					'iowait' 	=> $this->getIOWAIT(),
					'irq' 		=> $this->getIRQ(),
					'softirq' 	=> $this->getSoftIRQ(),
					'totalTime'	=> $this->getTotalTime(),
					'vss' 		=> $this->getVSS(),
					'rss' 		=> $this->getRSS(),
					'rlim' 		=> $this->getRLIM(),
					);
		
		return $data;
	}
}