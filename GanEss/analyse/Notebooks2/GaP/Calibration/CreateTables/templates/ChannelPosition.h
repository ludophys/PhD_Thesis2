DROP TABLE IF EXISTS `ChannelPosition`;
CREATE TABLE `ChannelPosition` (
	  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
	  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
	  `SensorID` int(11) NOT NULL COMMENT 'Hardware identifier of a specific sensor',
	  `Label` varchar(20) NOT NULL COMMENT 'Name of this sensor',
	  `Type` varchar(20) NOT NULL COMMENT 'Can be: "Cathode", "Anode" or "NaI"',
	  `X` float NOT NULL COMMENT 'Sensor x position (mm)',
	  `Y` float NOT NULL COMMENT 'Sensor y position (mm)'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
