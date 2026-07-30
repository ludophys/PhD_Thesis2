DROP TABLE IF EXISTS `PmtNoiseRms`;
CREATE TABLE `PmtNoiseRms` (
	  `MinRun` int(11) NOT NULL,
	  `MaxRun` int(11) DEFAULT NULL,
	  `ElecID` int(11) NOT NULL,
	  `noise_rms` double NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

