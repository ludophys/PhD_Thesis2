DROP TABLE IF EXISTS `PMTFEMapping`;
CREATE TABLE `PMTFEMapping` (
	  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number validity',
	  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number validity',
	  `SensorID` int(11) NOT NULL COMMENT 'Software sensor identifier',
	  `FEBox` int(11) NOT NULL COMMENT 'Front end box'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
