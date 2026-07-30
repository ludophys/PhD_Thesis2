DROP TABLE IF EXISTS `ChannelMask`;
CREATE TABLE `ChannelMask` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
  `SensorID` int(11) NOT NULL COMMENT 'Sensor identifier',
  KEY `ElecID` (`SensorID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

