DROP TABLE IF EXISTS `ChannelMapping`;
CREATE TABLE `ChannelMapping` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
  `ElecID` int(11) NOT NULL COMMENT 'Electronics identifier (eg, DAQ channel number)',
  `SensorID` int(11) NOT NULL COMMENT 'Hardware identifier of a specific sensor'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
